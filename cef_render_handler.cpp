#include "cef_render_handler.h"
#include "gstchromiumsrc.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_helpers.h>

#include <glib.h>

static GMutex cef_init_mutex;
static gboolean cef_initialized = FALSE;

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
        g_print("DEBUG: OnPaint #%d (page_loaded=%d)\n", paint_count, src_ ? src_->page_loaded : -1);

        if (!src_ || !src_->running || !src_->frame_buffer) {
            return;
        }

        if (width != width_ || height != height_) {
            g_warning("OnPaint: size mismatch: %dx%d vs %dx%d", width, height, width_, height_);
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
            g_print("DEBUG: Page loaded (HTTP %d)\n", httpStatusCode);
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
            g_warning("Load error: %s (%d) - %s", errorText.ToString().c_str(),
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

    while (src->running) {
        CefDoMessageLoopWork();
        count++;
        if (count % 60 == 0) {
            g_print("DEBUG: DoMessageLoopWork #%d\n", count);
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

    CefMainArgs main_args;
    CefSettings settings;

    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_WARNING;
    settings.multi_threaded_message_loop = FALSE;

    gchar *cache_dir = g_strdup_printf("/tmp/chromiumsrc-%d", getpid());
    g_mkdir_with_parents(cache_dir, 0700);
    CefString(&settings.root_cache_path) = cache_dir;
    CefString(&settings.cache_path) = cache_dir;
    g_free(cache_dir);

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
                g_print("DEBUG: Using CEF resources from: %s\n", resources_dir);
                g_free(icu_file);
                g_free(resources_dir);
                break;
            }
            g_free(icu_file);
        }
        g_free(resources_dir);
    }

    CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
    command_line->AppendSwitch("disable-extensions");
    command_line->AppendSwitch("disable-sync");
    command_line->AppendSwitch("disable-background-networking");
    command_line->AppendSwitch("no-first-run");
    command_line->AppendSwitchWithValue("log-severity", "warning");

    if (!g_getenv("DISPLAY")) {
        command_line->AppendSwitchWithValue("ozone-platform", "headless");
        command_line->AppendSwitchWithValue("headless", "new");
        command_line->AppendSwitch("disable-gpu");
        command_line->AppendSwitch("disable-gpu-compositing");
        command_line->AppendSwitch("disable-software-rasterizer");
        g_print("DEBUG: Running in headless mode (no DISPLAY)\n");
    }

    CefRefPtr<CefApp> app;

    if (!CefInitialize(main_args, settings, app, nullptr)) {
        g_mutex_unlock(&cef_init_mutex);
        g_warning("Failed to initialize CEF");
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
    if (!initialize_cef()) {
        return FALSE;
    }

    src->page_loaded = FALSE;

    CefRefPtr<CefRenderHandlerImpl> render_handler =
        new CefRenderHandlerImpl(src, width, height);

    CefRefPtr<CefLoadHandlerImpl> load_handler =
        new CefLoadHandlerImpl(src);

    CefRefPtr<CefLifeSpanHandlerImpl> lifespan_handler =
        new CefLifeSpanHandlerImpl(src);

    CefRefPtr<CefClientImpl> client =
        new CefClientImpl(render_handler, load_handler, lifespan_handler);

    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = src->fps_num;

    CefString cef_url(url);

    src->cef_client = static_cast<gpointer>(client.get());
    client->AddRef();

    if (!CefBrowserHost::CreateBrowser(
			window_info,
			client,
			cef_url,
            browser_settings,
			nullptr,
			nullptr)) {
        g_warning("Failed to create CEF browser");
        return FALSE;
    }

    // This line seems to not do anything?
    src->cef_thread = g_thread_new("cef-message-loop", cef_message_loop_thread, src);

    if (!src->cef_thread) {
        g_warning("Failed to create CEF message loop thread");
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
    if (!src) {
        return;
    }

    if (src->cef_thread) {
        g_thread_join(src->cef_thread);
        src->cef_thread = NULL;
    }

    if (src->cef_browser) {
        CefBrowser *browser = static_cast<CefBrowser *>(src->cef_browser);
        browser->GetHost()->CloseBrowser(TRUE);
        browser->Release();
        src->cef_browser = NULL;
    }

    if (src->cef_client) {
        CefClient *client = static_cast<CefClient *>(src->cef_client);
        client->Release();
        src->cef_client = NULL;
    }
}

}
