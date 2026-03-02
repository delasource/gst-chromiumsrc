#include "cef_render_handler.h"
#include "debug_utils.h"
#include "gpu_utils.h"
#include "gstchromiumsrc.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_helpers.h>

#include <glib.h>
#include <unistd.h>

static GMutex cef_init_mutex;
static gboolean cef_initialized = FALSE;
static GpuConfig *gpu_config = NULL;
static gboolean single_process_mode = FALSE;

void cef_set_single_process(gboolean single_process) {
    single_process_mode = single_process;
    DEBUG_LOG_CEF("set_single_process - Mode set to: %d", single_process);
}

/**
 * gpu_ensure_config:
 * @src: The GstChromiumSrc instance
 *
 * Ensures GPU configuration is initialized exactly once. Determines whether
 * to enable GPU acceleration based on user specification and system availability.
 *
 * Decision logic:
 * - If user explicitly enabled GPU and GPU is available: enable GPU
 * - If user explicitly disabled GPU: disable GPU
 * - If user didn't specify (auto): enable GPU if available
 */
static void gpu_ensure_config(GstChromiumSrc *src) {
    if (gpu_config) {
        DEBUG_LOG_GL("ensure_config - Config already exists, skipping");
        return;
    }

    gpu_config = gpu_config_new();

    if (src->gpu_user_specified) {
        if (src->gpu_enabled && gpu_is_available()) {
            gpu_config_detect(gpu_config);
            src->gpu_device = gpu_config->device_index;
        } else {
            gpu_config->enabled = FALSE;
            src->gpu_enabled = FALSE;
        }
    } else {
        if (gpu_is_available()) {
            gpu_config_detect(gpu_config);
            src->gpu_enabled = gpu_config->enabled;
            src->gpu_device = gpu_config->device_index;
        }
    }
    DEBUG_LOG_GL("ensure_config - Final config: enabled=%d, device=%s",
            src->gpu_enabled, gpu_config->device_path ? gpu_config->device_path : "none");
    
    // Log GL environment details
    //debug_log_gl_info();
}

/**
 * CefRenderHandlerImpl - Handles offscreen rendering for CEF browser
 *
 * Provides the view rectangle dimensions and receives painted pixel data
 * from CEF's offscreen rendering pipeline.
 */
class CefRenderHandlerImpl : public CefRenderHandler {
public:
    CefRenderHandlerImpl(GstChromiumSrc *src, int width, int height)
        : src_(src), width_(width), height_(height) {}

    /**
     * GetViewRect:
     * @browser: The CEF browser instance
     * @rect: Output parameter for the view rectangle
     *
     * Returns the dimensions of the offscreen rendering surface.
     * CEF calls this to determine the size of the buffer it should
     * allocate for rendering.
     *
     * Invoked by CEF when it needs to know the view dimensions,
     * typically during browser creation and resize operations.
     */
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override {
        rect.Set(0, 0, width_, height_);
    }

    /**
     * OnPaint:
     * @browser: The CEF browser instance
     * @type: The paint element type (view or popup)
     * @dirtyRects: List of rectangles that were repainted
     * @buffer: BGRA pixel data for the entire view
     * @width: Width of the buffer in pixels
     * @height: Height of the buffer in pixels
     *
     * Receives rendered pixel data from CEF and copies it to the
     * GStreamer element's frame buffer. Signals the frame_ready
     * condition to notify the need_data callback that a new frame
     * is available.
     *
     * Invoked by CEF after rendering a frame to the offscreen buffer.
     * Called on the CEF UI thread when page content changes.
     */
    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
        const RectList &dirtyRects, const void *buffer, int width, int height) override {

        static int paint_count = 0;
        paint_count++;

        if (!src_ || !src_->running || !src_->frame_buffer) {
            return;
        }

        if (width != width_ || height != height_) {
            DEBUG_LOG("OnPaint - Size mismatch: got %dx%d, expected %dx%d", 
                      width, height, width_, height_);
            return;
        }

        g_mutex_lock(&src_->frame_mutex);

        if (src_->running && src_->frame_buffer) {
            memcpy(src_->frame_buffer, buffer, src_->frame_size);
            src_->frame_ready = TRUE;
            g_cond_signal(&src_->frame_cond);
        }

        g_mutex_unlock(&src_->frame_mutex);
    }

private:
    GstChromiumSrc *src_;
    int width_;
    int height_;

    IMPLEMENT_REFCOUNTING(CefRenderHandlerImpl);
};

/**
 * CefLoadHandlerImpl - Handles page load events from CEF browser
 *
 * Monitors page loading progress and reports errors during navigation.
 */
class CefLoadHandlerImpl : public CefLoadHandler {
public:
    CefLoadHandlerImpl(GstChromiumSrc *src) : src_(src) {}

    /**
     * OnLoadEnd:
     * @browser: The CEF browser instance
     * @frame: The frame that finished loading
     * @httpStatusCode: HTTP status code of the response
     *
     * Marks the page as loaded when the main frame finishes loading.
     * This signals the message loop thread to start requesting paint
     * invalidations for continuous frame updates.
     *
     * Invoked by CEF when a frame completes loading, regardless of
     * success or failure.
     */
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        int httpStatusCode) override {
        if (frame->IsMain()) {
            DEBUG_LOG_CEF("Page loaded (HTTP %d)", httpStatusCode);
            src_->page_loaded = TRUE;
        }
    }

    /**
     * OnLoadError:
     * @browser: The CEF browser instance
     * @frame: The frame that encountered the error
     * @errorCode: The error code indicating failure type
     * @errorText: Human-readable error description
     * @failedUrl: The URL that failed to load
     *
     * Logs load errors for the main frame. Used for debugging
     * navigation issues.
     *
     * Invoked by CEF when a navigation fails (e.g., DNS failure,
     * connection refused, HTTP error).
     */
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        ErrorCode errorCode, const CefString &errorText, const CefString &failedUrl) override {
        if (frame->IsMain()) {
            DEBUG_LOG_CEF("Load error: %s (%d) - %s", errorText.ToString().c_str(),
                errorCode, failedUrl.ToString().c_str());
        }
    }

private:
    GstChromiumSrc *src_;
    IMPLEMENT_REFCOUNTING(CefLoadHandlerImpl);
};

/**
 * CefLifeSpanHandlerImpl - Handles browser lifecycle events
 *
 * Receives callbacks when a browser is created or closed.
 */
class CefLifeSpanHandlerImpl : public CefLifeSpanHandler {
public:
    CefLifeSpanHandlerImpl(GstChromiumSrc *src) : src_(src) {}

    /**
     * OnAfterCreated:
     * @browser: The newly created CEF browser instance
     *
     * Stores the browser reference after it's created. This is the
     * callback for the asynchronous CreateBrowser call.
     *
     * Invoked by CEF after the browser has been created successfully.
     */
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        CEF_REQUIRE_UI_THREAD();
        if (src_) {
            src_->cef_browser = static_cast<gpointer>(browser.get());
            browser->AddRef();
        }
    }

private:
    GstChromiumSrc *src_;
    IMPLEMENT_REFCOUNTING(CefLifeSpanHandlerImpl);
};

/**
 * CefClientImpl - Main CEF client interface implementation
 *
 * Provides access to the render, load, and lifespan handlers. This is the main
 * interface CEF uses to communicate with the application.
 */
class CefClientImpl : public CefClient {
public:
    CefClientImpl(CefRefPtr<CefRenderHandler> render_handler,
                  CefRefPtr<CefLoadHandler> load_handler,
                  CefRefPtr<CefLifeSpanHandler> lifespan_handler)
        : render_handler_(render_handler),
          load_handler_(load_handler),
          lifespan_handler_(lifespan_handler) {}

    /**
     * GetRenderHandler:
     *
     * Returns the render handler for offscreen rendering.
     *
     * Invoked by CEF when it needs to perform offscreen rendering
     * operations.
     */
    CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }

    /**
     * GetLoadHandler:
     *
     * Returns the load handler for monitoring page load events.
     *
     * Invoked by CEF when a navigation begins or ends.
     */
    CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return load_handler_;
    }

    /**
     * GetLifeSpanHandler:
     *
     * Returns the lifespan handler for browser creation callbacks.
     *
     * Invoked by CEF during browser creation and destruction.
     */
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return lifespan_handler_;
    }

private:
    CefRefPtr<CefRenderHandler> render_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefLifeSpanHandler> lifespan_handler_;

    IMPLEMENT_REFCOUNTING(CefClientImpl);
};

/**
 * cef_message_loop_thread:
 * @data: The GstChromiumSrc instance
 *
 * Thread function that runs the CEF message loop. Continuously calls
 * CefDoMessageLoopWork() to process CEF events and periodically
 * invalidates the view to trigger repaints when the page is loaded.
 *
 * Runs in a separate GLib thread created in cef_browser_start().
 * Exits when src->running becomes FALSE.
 *
 * Returns: NULL always
 */
static gpointer cef_message_loop_thread(gpointer data) {
    GstChromiumSrc *src = (GstChromiumSrc *)data;
    int count = 0;

    DEBUG_LOG_CEF("message_loop_thread - Thread started (tid=%d)", gettid());

    while (src->running) {
        CefDoMessageLoopWork();
        count++;
        if (count % 60 == 0) {
            DEBUG_LOG_CEF("message_loop - Iteration #%d (page_loaded=%d, browser=%p)", 
                    count, src->page_loaded, src->cef_browser);
        }

        if (src->page_loaded && src->cef_browser) {
            CefBrowser *browser = static_cast<CefBrowser *>(src->cef_browser);
            browser->GetHost()->Invalidate(PET_VIEW);
        }

        g_usleep(10000);
    }

    return NULL;
}

/**
 * initialize_cef:
 *
 * Initializes the CEF framework. Sets up single-process mode with
 * windowless rendering disabled GPU acceleration. Uses thread-safe
 * initialization to prevent multiple calls.
 *
 * Invoked by cef_browser_start() before creating a browser instance.
 * Safe to call multiple times; subsequent calls are no-ops.
 *
 * Returns: TRUE on success, FALSE on failure
 */
static gboolean initialize_cef(void) {
    g_mutex_lock(&cef_init_mutex);

    if (cef_initialized) {
        g_mutex_unlock(&cef_init_mutex);
        return TRUE;
    }

    /**
     * CefAppImpl - CEF application handler for command line processing
     *
     * Configures Chromium command line switches before browser creation.
     * Handles GPU mode selection and headless rendering configuration.
     */
    class CefAppImpl : public CefApp, public CefBrowserProcessHandler {
    public:
        CefAppImpl() {}

        /**
         * OnBeforeCommandLineProcessing:
         * @process_type: Empty for browser process, type for subprocess
         * @command_line: The command line to modify
         *
         * Called before CEF processes command line arguments.
         * This is where we inject Chromium flags for GPU/headless modes.
         */
        void OnBeforeCommandLineProcessing(
            const CefString& process_type,
            CefRefPtr<CefCommandLine> command_line) override {

            // Disable unnecessary features for headless rendering
            command_line->AppendSwitch("disable-extensions");
            command_line->AppendSwitch("disable-sync");
            command_line->AppendSwitch("disable-background-networking");
            command_line->AppendSwitch("no-first-run");
            
            // Security: disable GPU sandbox for headless rendering
            command_line->AppendSwitch("disable-gpu-sandbox");
            command_line->AppendSwitch("disable-seccomp-filter-sandbox");
            command_line->AppendSwitch("no-sandbox");
            command_line->AppendSwitchWithValue("log-severity", "warning");

            // Handle single-process mode (used for debugging/easier deployment)
            if (single_process_mode) {
                command_line->AppendSwitch("single-process");
                command_line->AppendSwitch("disable-zygote");
                command_line->AppendSwitch("in-process-gpu");
            }

            // Determine GPU mode based on config and environment
            gboolean has_display = g_getenv("DISPLAY") != NULL;
            gboolean should_enable_gpu = (gpu_config && gpu_config->enabled) || 
                                          (!gpu_config && gpu_is_available());

            if (should_enable_gpu) {
                // GPU-accelerated rendering path
                if (gpu_config && gpu_config->device_path) {
                    DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU device: %s", 
                            gpu_config->device_path);
                } else {
                    DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU device: subprocess auto-detect");
                }
                
                // GL implementation selection
                command_line->AppendSwitchWithValue("use-gl", "egl-angle");
                command_line->AppendSwitchWithValue("use-angle", "egl");
                command_line->AppendSwitch("enable-gpu-rasterization");
                command_line->AppendSwitch("enable-zero-copy");
                command_line->AppendSwitch("ignore-gpu-blocklist");

                if (!has_display) {
                    DEBUG_LOG_GL("OnBeforeCommandLineProcessing - Headless GPU mode (no DISPLAY)");
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                } else {
                    DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU mode with DISPLAY");
                }
            } else if (!has_display) {
                // CPU-only headless rendering path
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - CPU headless mode (no DISPLAY, no GPU)");
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("disable-gpu-compositing");
                command_line->AppendSwitch("disable-software-rasterizer");
            } else {
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - CPU mode with display");
            }
        }

        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
            return this;
        }

        /**
         * OnBeforeChildProcessLaunch:
         * @command_line: The command line that will be passed to the child process
         *
         * Called before spawning a child process. This is where we pass GL-related
         * switches to subprocesses (renderer, GPU, utility) so they use the same
         * GL implementation as the browser process.
         *
         * Without this, subprocesses get gl=none,angle=none and fail to initialize.
         */
        void OnBeforeChildProcessLaunch(
            CefRefPtr<CefCommandLine> command_line) override {

            DEBUG_LOG_CEF("OnBeforeChildProcessLaunch - Configuring subprocess command line");

            // Pass GPU-related switches to child processes
            gboolean has_display = g_getenv("DISPLAY") != NULL;
            gboolean should_enable_gpu = (gpu_config && gpu_config->enabled) || 
                                          (!gpu_config && gpu_is_available());

            if (should_enable_gpu) {
                DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Adding GL switches for GPU subprocess");
                command_line->AppendSwitchWithValue("use-gl", "egl-angle");
                command_line->AppendSwitchWithValue("use-angle", "egl");
                command_line->AppendSwitch("enable-gpu-rasterization");
                command_line->AppendSwitch("ignore-gpu-blocklist");

                if (!has_display) {
                    DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Adding headless switches");
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                }
            } else if (!has_display) {
                DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Adding CPU headless switches");
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("disable-gpu-compositing");
            }

            // Always pass security/sandbox switches
            command_line->AppendSwitch("disable-gpu-sandbox");
            command_line->AppendSwitch("disable-seccomp-filter-sandbox");
            command_line->AppendSwitch("no-sandbox");
            command_line->AppendSwitchWithValue("log-severity", "warning");

            DEBUG_LOG_CEF("OnBeforeChildProcessLaunch - Subprocess command line configured");
        }

        IMPLEMENT_REFCOUNTING(CefAppImpl);
    };

    // Create CEF app instance
    CefMainArgs main_args;
    CefRefPtr<CefAppImpl> app = new CefAppImpl();

    // Handle subprocess execution in multi-process mode
    if (!single_process_mode) {
        int exit_code = CefExecuteProcess(main_args, app, nullptr);
        if (exit_code >= 0) {
            DEBUG_LOG_CEF("initialize_cef - Running as subprocess, exiting with code %d", exit_code);
            _exit(exit_code);
        }
        DEBUG_LOG_CEF("initialize_cef - Running as main browser process");
    }

    // Configure CEF settings
    CefSettings settings;
    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_WARNING;
    settings.multi_threaded_message_loop = FALSE;
    CefString(&settings.browser_subprocess_path) = "/usr/local/lib/chromiumsrc-subprocess";

    // Create unique cache directory for this process
    gchar *cache_dir = g_strdup_printf("/tmp/chromiumsrc-%d", getpid());
    g_mkdir_with_parents(cache_dir, 0700);
    CefString(&settings.root_cache_path) = cache_dir;
    CefString(&settings.cache_path) = cache_dir;
    g_free(cache_dir);

    // Search for CEF resources (locales, ICU data, etc.)
    const gchar *search_paths[] = {
        g_getenv("CHROMIUMSRC_RESOURCES_PATH"),
        g_getenv("GST_PLUGIN_PATH"),
        "/usr/local/lib/gstreamer-1.0",
        "/usr/lib/gstreamer-1.0",
        "/usr/lib/x86_64-linux-gnu/gstreamer-1.0",
        g_getenv("HOME") ? g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins", g_getenv("HOME")) : NULL,
        NULL
    };

    for (int i = 0; search_paths[i] != NULL; i++) {
        gchar *resources_dir = g_strdup_printf("%s/Resources", search_paths[i]);
        if (g_file_test(resources_dir, G_FILE_TEST_IS_DIR)) {
            gchar *icu_file = g_strdup_printf("%s/icudtl.dat", resources_dir);
            if (g_file_test(icu_file, G_FILE_TEST_EXISTS)) {
                CefString(&settings.resources_dir_path) = resources_dir;
                g_free(icu_file);
                g_free(resources_dir);
                break;
            }
            g_free(icu_file);
        }
        g_free(resources_dir);
    }

    // Initialize CEF
    if (!CefInitialize(main_args, settings, app, nullptr)) {
        g_mutex_unlock(&cef_init_mutex);
        DEBUG_LOG_CEF("initialize_cef - CefInitialize FAILED");
        return FALSE;
    }

    cef_initialized = TRUE;
    g_mutex_unlock(&cef_init_mutex);

    return TRUE;
}


extern "C" {

/**
 * cef_browser_start:
 * @src: The GstChromiumSrc instance
 * @url: The URL to load in the browser
 * @width: Width of the rendering surface in pixels
 * @height: Height of the rendering surface in pixels
 *
 * Creates and starts a CEF browser instance for offscreen rendering.
 * Initializes CEF if not already done, creates handlers, configures
 * windowless rendering, and launches the message loop thread.
 *
 * Invoked by gst_chromium_src_start() during the READY_TO_PAUSED
 * state transition.
 *
 * Returns: TRUE on success, FALSE on failure
 */
gboolean cef_browser_start(GstChromiumSrc *src, const gchar *url, gint width, gint height) {
    // Step 1: Configure GPU acceleration
    gpu_ensure_config(src);
    cef_set_single_process(src->single_process);

    // Step 2: Initialize CEF framework
    if (!initialize_cef()) {
        DEBUG_LOG("cef_browser_start - CEF initialization FAILED");
        return FALSE;
    }

    src->page_loaded = FALSE;

    // Step 3: Create CEF handlers
    CefRefPtr<CefRenderHandlerImpl> render_handler =
        new CefRenderHandlerImpl(src, width, height);

    CefRefPtr<CefLoadHandlerImpl> load_handler =
        new CefLoadHandlerImpl(src);

    CefRefPtr<CefLifeSpanHandlerImpl> lifespan_handler =
        new CefLifeSpanHandlerImpl(src);

    // Step 4: Create CEF client
    CefRefPtr<CefClientImpl> client =
        new CefClientImpl(render_handler, load_handler, lifespan_handler);

    // Step 5: Configure windowless rendering
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    // Step 6: Configure browser settings
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = src->fps_num;

    CefString cef_url(url);

    // Step 7: Store client reference
    src->cef_client = static_cast<gpointer>(client.get());
    client->AddRef();

    // Step 8: Create browser asynchronously
    if (!CefBrowserHost::CreateBrowser(
			window_info,
			client,
			cef_url,
            browser_settings,
			nullptr,
			nullptr)) {
        DEBUG_LOG("cef_browser_start - CreateBrowser FAILED");
        return FALSE;
    }

    // Step 9: Start message loop thread
    src->cef_thread = g_thread_new("cef-message-loop", cef_message_loop_thread, src);

    if (!src->cef_thread) {
        DEBUG_LOG("cef_browser_start - Failed to create message loop thread");
        return FALSE;
    }

    return TRUE;
}

/**
 * cef_browser_stop:
 * @src: The GstChromiumSrc instance
 *
 * Stops and cleans up the CEF browser instance. Signals the message
 * loop thread to stop, waits for it to join, closes the browser,
 * and releases all CEF references.
 *
 * Invoked by gst_chromium_src_stop() during the PAUSED_TO_READY
 * state transition.
 */
void cef_browser_stop(GstChromiumSrc *src) {
    DEBUG_LOG("=== CEF Browser Stop ===");
    
    if (!src) {
        DEBUG_LOG("cef_browser_stop - Source is NULL, nothing to stop");
        return;
    }

    // Step 1: Stop message loop thread
    if (src->cef_thread) {
        g_thread_join(src->cef_thread);
        src->cef_thread = NULL;
        DEBUG_LOG("cef_browser_stop - CEF thread joined and cleared");
    } else {
        DEBUG_LOG("cef_browser_stop - No CEF thread to join");
    }

    // Step 2: Close browser
    if (src->cef_browser) {
        CefBrowser *browser = static_cast<CefBrowser *>(src->cef_browser);
        browser->GetHost()->CloseBrowser(TRUE);
        browser->Release();
        src->cef_browser = NULL;
        DEBUG_LOG("cef_browser_stop - Browser closed and released");
    } else {
        DEBUG_LOG("cef_browser_stop - No browser to close");
    }

    // Step 3: Release client
    if (src->cef_client) {
        CefClient *client = static_cast<CefClient *>(src->cef_client);
        client->Release();
        src->cef_client = NULL;
        DEBUG_LOG("cef_browser_stop - Client released");
    } else {
        DEBUG_LOG("cef_browser_stop - No client to release");
    }
}

}
