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
#include <vector>

static GMutex cef_init_mutex;
static gboolean cef_initialized = FALSE;
static GpuConfig* gpu_config = NULL;
static guint cef_idle_id = 0;
static int cef_message_count = 0;

static gboolean cef_message_loop_idle(gpointer data);

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
static void gpu_ensure_config(GstChromiumSrc* src)
{
    if (gpu_config)
    {
        DEBUG_LOG_GL("ensure_config - Config already exists, skipping");
        return;
    }

    gpu_config = gpu_config_new();

    if (src->gpu_user_specified)
    {
        if (src->gpu_enabled && gpu_is_available())
        {
            gpu_config_detect(gpu_config);
            src->gpu_device = gpu_config->device_index;
        }
        else
        {
            gpu_config->enabled = FALSE;
            src->gpu_enabled = FALSE;
        }
    }
    else
    {
        if (gpu_is_available())
        {
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
class CefRenderHandlerImpl : public CefRenderHandler
{
public:
    CefRenderHandlerImpl(GstChromiumSrc* src, int width, int height)
        : src_(src), width_(width), height_(height)
    {
    }

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
    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
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
    void OnPaint(CefRefPtr<CefBrowser> browser,
                 PaintElementType type,
                 const RectList& dirtyRects,
                 const void* buffer,
                 int width,
                 int height) override
    {
        static int paint_count = 0;
        paint_count++;

        if (!src_ || !src_->running || !src_->frame_buffer)
        {
            return;
        }

        if (width != width_ || height != height_)
        {
            DEBUG_LOG("OnPaint - Size mismatch: got %dx%d, expected %dx%d",
                      width, height, width_, height_);
            return;
        }

        g_mutex_lock(&src_->frame_mutex);

        memcpy(src_->frame_buffer, buffer, src_->frame_size);
        src_->frame_ready = TRUE;
        g_cond_signal(&src_->frame_cond);

        g_mutex_unlock(&src_->frame_mutex);
    }

private:
    GstChromiumSrc* src_;
    int width_;
    int height_;

    IMPLEMENT_REFCOUNTING(CefRenderHandlerImpl);
};

/**
 * CefLoadHandlerImpl - Handles page load events from CEF browser
 *
 * Monitors page loading progress and reports errors during navigation.
 */
class CefLoadHandlerImpl : public CefLoadHandler
{
public:
    CefLoadHandlerImpl(GstChromiumSrc* src) : src_(src)
    {
    }

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
    void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                   CefRefPtr<CefFrame> frame,
                   int httpStatusCode) override
    {
        if (frame->IsMain())
        {
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
    void OnLoadError(CefRefPtr<CefBrowser> browser,
                     CefRefPtr<CefFrame> frame,
                     ErrorCode errorCode,
                     const CefString& errorText,
                     const CefString& failedUrl) override
    {
        if (frame->IsMain())
        {
            DEBUG_LOG_CEF("Load error: %s (%d) - %s", errorText.ToString().c_str(),
                          errorCode, failedUrl.ToString().c_str());
        }
    }

private:
    GstChromiumSrc* src_;
    IMPLEMENT_REFCOUNTING(CefLoadHandlerImpl);
};

/**
 * CefLifeSpanHandlerImpl - Handles browser lifecycle events
 *
 * Receives callbacks when a browser is created or closed.
 */
class CefLifeSpanHandlerImpl : public CefLifeSpanHandler
{
public:
    CefLifeSpanHandlerImpl(GstChromiumSrc* src) : src_(src)
    {
    }

    /**
     * OnAfterCreated:
     * @browser: The newly created CEF browser instance
     *
     * Stores the browser reference after it's created. This is the
     * callback for the asynchronous CreateBrowser call.
     *
     * Invoked by CEF after the browser has been created successfully.
     */
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        DEBUG_LOG_CEF("OnAfterCreated called - browser=%p, src_=%p", browser.get(), src_);
        CEF_REQUIRE_UI_THREAD();
        if (src_)
        {
            src_->cef_browser = static_cast<gpointer>(browser.get());
            browser->AddRef();
            DEBUG_LOG_CEF("OnAfterCreated - Browser stored successfully");
        }
        else
        {
            DEBUG_LOG_CEF("OnAfterCreated - WARNING: src_ is NULL!");
        }
    }

private:
    GstChromiumSrc* src_;
    IMPLEMENT_REFCOUNTING(CefLifeSpanHandlerImpl);
};

/**
 * CefClientImpl - Main CEF client interface implementation
 *
 * Provides access to the render, load, and lifespan handlers. This is the main
 * interface CEF uses to communicate with the application.
 */
class CefClientImpl : public CefClient
{
public:
    CefClientImpl(CefRefPtr<CefRenderHandler> render_handler,
                  CefRefPtr<CefLoadHandler> load_handler,
                  CefRefPtr<CefLifeSpanHandler> lifespan_handler)
        : render_handler_(render_handler),
          load_handler_(load_handler),
          lifespan_handler_(lifespan_handler)
    {
    }

    /**
     * GetRenderHandler:
     *
     * Returns the render handler for offscreen rendering.
     *
     * Invoked by CEF when it needs to perform offscreen rendering
     * operations.
     */
    CefRefPtr<CefRenderHandler> GetRenderHandler() override
    {
        return render_handler_;
    }

    /**
     * GetLoadHandler:
     *
     * Returns the load handler for monitoring page load events.
     *
     * Invoked by CEF when a navigation begins or ends.
     */
    CefRefPtr<CefLoadHandler> GetLoadHandler() override
    {
        return load_handler_;
    }

    /**
     * GetLifeSpanHandler:
     *
     * Returns the lifespan handler for browser creation callbacks.
     *
     * Invoked by CEF during browser creation and destruction.
     */
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
    {
        return lifespan_handler_;
    }

private:
    CefRefPtr<CefRenderHandler> render_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefLifeSpanHandler> lifespan_handler_;

    IMPLEMENT_REFCOUNTING(CefClientImpl);
};

static gboolean cef_message_loop_idle(gpointer data)
{
    const auto src = static_cast<GstChromiumSrc*>(data);

    if (!src->running)
    {
        cef_idle_id = 0;
        return G_SOURCE_REMOVE;
    }

    CefDoMessageLoopWork();
    cef_message_count++;

    // Invalidate at target frame rate (every ~33ms for 30fps)
    // Message loop runs every 10ms, so invalidate every 3rd iteration
    if (src->page_loaded && src->cef_browser && (cef_message_count % 3 == 0))
    {
        auto browser = static_cast<CefBrowser*>(src->cef_browser);
        browser->GetHost()->Invalidate(PET_VIEW);
    }

    return G_SOURCE_CONTINUE;
}

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
/*static gpointer cef_message_loop_thread(gpointer data)
{
    const auto src = static_cast<GstChromiumSrc*>(data);
    int count = 0;

    while (src->running)
    {
        CefDoMessageLoopWork();
        count++;
        if (count % 300 == 0)
        {
            DEBUG_LOG_CEF("message_loop running (5s interval ping) Iteration #%d (page_loaded=%d, browser=%p)",
                          count, src->page_loaded, src->cef_browser);
        }

        if (src->page_loaded && src->cef_browser)
        {
            auto browser = static_cast<CefBrowser*>(src->cef_browser);
            browser->GetHost()->Invalidate(PET_VIEW);
        }

        g_usleep(10000);
    }

    return nullptr;
}*/


/**
 * initialize_cef:
 *
 * Invoked by cef_browser_start() before creating a browser instance.
 * Safe to call multiple times; subsequent calls are no-ops.
 *
 * Returns: TRUE on success, FALSE on failure
 */
static gboolean initialize_cef()
{
    g_mutex_lock(&cef_init_mutex);

    if (cef_initialized)
    {
        g_mutex_unlock(&cef_init_mutex);
        return TRUE;
    }

    /**
     * CefAppImpl - CEF application handler for command line processing
     *
     * Configures Chromium command line switches before browser creation.
     * Handles GPU mode selection and headless rendering configuration.
     */
    class CefAppImpl : public CefApp, public CefBrowserProcessHandler
    {
    public:
        CefAppImpl()
        {
        }

        /**
         * OnBeforeCommandLineProcessing:
         * @process_type: Empty for browser process, type for subprocess
         * @command_line: The command line to modify
         *
         * Called before CEF processes command line arguments.
         * This is where we inject Chromium flags for GPU/headless modes.
         */
        void OnBeforeCommandLineProcessing(const CefString& process_type,
                                           const CefRefPtr<CefCommandLine> command_line) override
        {
            command_line->AppendSwitch("disable-extensions");
            command_line->AppendSwitch("disable-sync");
            command_line->AppendSwitch("disable-background-networking");
            command_line->AppendSwitch("no-first-run");

            command_line->AppendSwitch("disable-gpu-sandbox");
            command_line->AppendSwitch("disable-seccomp-filter-sandbox");
            command_line->AppendSwitch("no-sandbox");

            const gchar* display = g_getenv("DISPLAY");
            gboolean has_display = display != nullptr && 
                                   g_strcmp0(display, "NULL") != 0 &&
                                   strlen(display) > 0;
            gboolean should_enable_gpu = (gpu_config && gpu_config->enabled) ||
                (!gpu_config && gpu_is_available());

            if (should_enable_gpu)
            {
                //DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Disabling GPU, using software compositing");
                //command_line->AppendSwitch("disable-gpu");
                //command_line->AppendSwitch("disable-software-rasterizer");
                //command_line->AppendSwitchWithValue("num-raster-threads", "4");
                command_line->AppendSwitchWithValue("use-gl", "egl-angle");
                command_line->AppendSwitchWithValue("use-angle", "egl");
                command_line->AppendSwitch("enable-gpu-rasterization");
                command_line->AppendSwitch("enable-zero-copy");
                command_line->AppendSwitch("ignore-gpu-blocklist");

                if (!has_display)
                {
                    DEBUG_LOG_GL("OnBeforeCommandLineProcessing - NO DISPLAY");
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                }
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU mode enabled");
            }
            else
            {
                // Use SwiftShader for software rendering instead of disabling GPU entirely
                command_line->AppendSwitchWithValue("use-gl", "swiftshader");
                command_line->AppendSwitch("disable-gpu");
                command_line->AppendSwitch("in-process-gpu");

                if (!has_display)
                {
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                }
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - SwiftShader software rendering");
            }
        }

        CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
        {
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
        void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override
        {
            //DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Initial: %s", command_line->GetCommandLineString().ToString().c_str());

            const gchar* display = g_getenv("DISPLAY");
            gboolean has_display = display != NULL &&
                                   g_strcmp0(display, "NULL") != 0 &&
                                   strlen(display) > 0;
            gboolean should_enable_gpu = (gpu_config && gpu_config->enabled) ||
                (!gpu_config && gpu_is_available());


            if (should_enable_gpu)
            {
                command_line->AppendSwitchWithValue("use-gl", "egl-angle");
                command_line->AppendSwitchWithValue("use-angle", "egl");
                command_line->AppendSwitch("enable-gpu-rasterization");
                command_line->AppendSwitch("ignore-gpu-blocklist");
                //DEBUG_LOG_GL("OnBeforeCommandLineProcessing - Disabling GPU, using software compositing");
                //command_line->AppendSwitch("disable-gpu");
                //command_line->AppendSwitch("disable-software-rasterizer");
                //command_line->AppendSwitchWithValue("num-raster-threads", "4");

                if (!has_display)
                {
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                }
            }
            else
            {
                command_line->AppendSwitchWithValue("use-gl", "swiftshader");
                command_line->AppendSwitchWithValue("use-angle", "swiftshader");

                if (!has_display)
                {
                    command_line->AppendSwitchWithValue("ozone-platform", "headless");
                    command_line->AppendSwitchWithValue("headless", "new");
                }
            }

            command_line->AppendSwitch("disable-gpu-sandbox");
            command_line->AppendSwitch("disable-seccomp-filter-sandbox");
            command_line->AppendSwitch("no-sandbox");

            DEBUG_LOG_GL("OnBeforeChildProcessLaunch - Final: %s", 
                         command_line->GetCommandLineString().ToString().c_str());
        }

        IMPLEMENT_REFCOUNTING(CefAppImpl);
    };

    // Create CEF app instance
    CefMainArgs main_args;
    CefRefPtr<CefAppImpl> app = new CefAppImpl();

    DEBUG_LOG_CEF("=== Environment Variables ===");
    DEBUG_LOG_CEF("XDG_RUNTIME_DIR: %s", g_getenv("XDG_RUNTIME_DIR") ? g_getenv("XDG_RUNTIME_DIR") : "(not set)");
    DEBUG_LOG_CEF("DBUS_SESSION_BUS_ADDRESS: %s",
                  g_getenv("DBUS_SESSION_BUS_ADDRESS") ? g_getenv("DBUS_SESSION_BUS_ADDRESS") : "(not set)");
    DEBUG_LOG_CEF("HOME: %s", g_getenv("HOME") ? g_getenv("HOME") : "(not set)");
    DEBUG_LOG_CEF("XAUTHORITY: %s", g_getenv("XAUTHORITY") ? g_getenv("XAUTHORITY") : "(not set)");
    DEBUG_LOG_CEF("=============================");

    CefSettings settings;
    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_INFO;
    settings.multi_threaded_message_loop = FALSE;

    // Disable GPU if configured
    if (gpu_config && !gpu_config->enabled)
    {
        settings.chrome_runtime = FALSE;
    }

    const gchar* env_subprocess = g_getenv("CHROMIUMSRC_SUBPROCESS_PATH");
    const gchar* home_dir = g_getenv("HOME");

    gchar* home_subprocess = home_dir
                                 ? g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins/chromiumsrc-subprocess",
                                                   home_dir)
                                 : nullptr;

    std::vector<const gchar*> subprocess_paths;
    if (env_subprocess) subprocess_paths.push_back(env_subprocess);
    if (home_subprocess) subprocess_paths.push_back(home_subprocess);
    subprocess_paths.push_back("/usr/local/lib/gstreamer-1.0/chromiumsrc-subprocess");
    subprocess_paths.push_back("/usr/lib/gstreamer-1.0/chromiumsrc-subprocess");
    subprocess_paths.push_back(nullptr);

    const gchar* found_subprocess = nullptr;
    for (size_t i = 0; subprocess_paths[i] != nullptr; i++)
    {
        if (g_file_test(subprocess_paths[i], G_FILE_TEST_EXISTS))
        {
            found_subprocess = subprocess_paths[i];
            DEBUG_LOG_CEF("initialize_cef - Found subprocess: %s", found_subprocess);
            break;
        }
        else
        {
            DEBUG_LOG_CEF("initialize_cef - Path not found: %s", subprocess_paths[i]);
        }
    }

    if (!found_subprocess)
    {
        g_free(home_subprocess);
        g_mutex_unlock(&cef_init_mutex);
        g_critical("chromiumsrc: subprocess binary 'chromiumsrc-subprocess' not found. "
            "Set CHROMIUMSRC_SUBPROCESS_PATH or install to ~/.local/share/gstreamer-1.0/plugins/");
        DEBUG_LOG_CEF("initialize_cef - FAILED: subprocess binary not found");
        return FALSE;
    }

    CefString(&settings.browser_subprocess_path) = found_subprocess;
    g_free(home_subprocess);

    // Create unique cache directory for this process
    gchar* cache_dir = g_strdup_printf("/tmp/chromiumsrc-%d", getpid());
    g_mkdir_with_parents(cache_dir, 0700);
    CefString(&settings.root_cache_path) = cache_dir;
    CefString(&settings.cache_path) = cache_dir;
    g_free(cache_dir);

    // Search for CEF resources (locales, ICU data, etc.)
    const gchar* search_paths[] = {
        g_getenv("CHROMIUMSRC_RESOURCES_PATH") ? g_getenv("CHROMIUMSRC_RESOURCES_PATH") : "skip",
        g_getenv("GST_PLUGIN_PATH") ? g_strdup_printf("%s/gstreamer-1.0", g_getenv("GST_PLUGIN_PATH")) : "skip",
        g_getenv("HOME") ? g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins", g_getenv("HOME")) : "skip",
        "/usr/local/lib/gstreamer-1.0",
        "/usr/lib/gstreamer-1.0",
        "/usr/lib/x86_64-linux-gnu/gstreamer-1.0",
        NULL
    };

    for (int i = 0; search_paths[i] != NULL; i++)
    {
        if (g_strcmp0(search_paths[i], "skip") == 0)
        {
            continue;
        }
        gchar* resources_dir = g_strdup_printf("%s/Resources", search_paths[i]);
        if (g_file_test(resources_dir, G_FILE_TEST_IS_DIR))
        {
            gchar* icu_file = g_strdup_printf("%s/icudtl.dat", resources_dir);
            if (g_file_test(icu_file, G_FILE_TEST_EXISTS))
            {
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
    if (!CefInitialize(main_args, settings, app, nullptr))
    {
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
gboolean cef_browser_start(GstChromiumSrc* src, const gchar* url, gint width, gint height)
{
    gpu_ensure_config(src);

    if (!initialize_cef())
    {
        DEBUG_LOG("cef_browser_start - CEF initialization FAILED");
        return FALSE;
    }

    // Pump message loop a few times to let CEF finish initialization
    for (int i = 0; i < 10; i++)
    {
        CefDoMessageLoopWork();
        g_usleep(1000);
    }
    DEBUG_LOG_CEF("CEF initialization complete, creating browser...");

    src->page_loaded = FALSE;

    // Step 3: Create CEF handlers
    CefRefPtr<CefRenderHandlerImpl> render_handler = new CefRenderHandlerImpl(src, width, height);

    CefRefPtr<CefLoadHandlerImpl> load_handler = new CefLoadHandlerImpl(src);

    CefRefPtr<CefLifeSpanHandlerImpl> lifespan_handler = new CefLifeSpanHandlerImpl(src);

    // Step 4: Create CEF client
    CefRefPtr<CefClientImpl> client = new CefClientImpl(render_handler, load_handler, lifespan_handler);

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

    // Step 8: Start message loop via GLib idle callback (runs on main thread)
    src->running = TRUE;
    cef_idle_id = g_timeout_add(8, cef_message_loop_idle, src);
    DEBUG_LOG_CEF("Started CEF message loop timeout callback (id=%u)", cef_idle_id);

    // Step 9: Create browser asynchronously
    DEBUG_LOG_CEF("CreateBrowser - url=%s, client=%p, windowless=%d",
                  url, client.get(), window_info.windowless_rendering_enabled);

    if (!CefBrowserHost::CreateBrowser(
        window_info,
        client,
        cef_url,
        browser_settings,
        nullptr,
        nullptr))
    {
        DEBUG_LOG_CEF("cef_browser_start - CreateBrowser FAILED");
        src->running = FALSE;
        if (cef_idle_id)
        {
            g_source_remove(cef_idle_id);
            cef_idle_id = 0;
        }
        return FALSE;
    }

    DEBUG_LOG_CEF("CreateBrowser - Initiated successfully, waiting for OnAfterCreated...");

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
void cef_browser_stop(GstChromiumSrc* src)
{
    DEBUG_LOG("=== CEF Browser Stop ===");

    if (!src)
    {
        DEBUG_LOG("cef_browser_stop - Source is NULL, nothing to stop");
        return;
    }

    // Step 1: Stop message loop idle callback
    src->running = FALSE;
    if (cef_idle_id)
    {
        g_source_remove(cef_idle_id);
        cef_idle_id = 0;
        DEBUG_LOG("cef_browser_stop - CEF idle callback removed");
    }

    // Step 2: Close browser
    if (src->cef_browser)
    {
        CefBrowser* browser = static_cast<CefBrowser*>(src->cef_browser);
        browser->GetHost()->CloseBrowser(TRUE);
        browser->Release();
        src->cef_browser = NULL;
        DEBUG_LOG("cef_browser_stop - Browser closed and released");
    }
    else
    {
        DEBUG_LOG("cef_browser_stop - No browser to close");
    }

    // Step 3: Release client
    if (src->cef_client)
    {
        CefClient* client = static_cast<CefClient*>(src->cef_client);
        client->Release();
        src->cef_client = NULL;
        DEBUG_LOG("cef_browser_stop - Client released");
    }
    else
    {
        DEBUG_LOG("cef_browser_stop - No client to release");
    }
}
}
