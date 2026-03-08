#include "cef_manager.h"
#include "debug_utils.h"
#include "gpu_utils.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>
#include <include/cef_command_line.h>
#include <include/wrapper/cef_helpers.h>

#include <unistd.h>
#include <vector>

// ============================================================================
// BrowserInstance - Internal browser tracking structure
// ============================================================================

struct _BrowserInstance
{
    CefRefPtr<CefBrowser> cef_browser;
    BrowserCallbacks callbacks;
    gint width;
    gint height;
    gint fps;
    gboolean page_loaded;
    gboolean running;
};

// ============================================================================
// Static initialization (thread-safe in C++11)
// ============================================================================

CefManager* CefManager::instance_ = nullptr;
GMutex CefManager::init_mutex_;

// ============================================================================
// CefApp implementation - Handles command line processing
// ============================================================================

/**
 * CefManagerApp - CEF application handler for the browser process
 *
 * Configures Chromium command line switches before browser creation.
 * Handles GPU mode selection and headless rendering configuration.
 */
class CefManagerApp : public CefApp, public CefBrowserProcessHandler
{
public:
    CefManagerApp(gboolean gpu_enabled, gint gpu_device)
        : gpu_enabled_(gpu_enabled), gpu_device_(gpu_device)
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
    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        const CefRefPtr<CefCommandLine> command_line) override
    {
        // Basic stability switches
        command_line->AppendSwitch("disable-extensions");
        command_line->AppendSwitch("disable-sync");
        command_line->AppendSwitch("disable-background-networking");
        command_line->AppendSwitch("no-first-run");

        // Sandbox settings
        command_line->AppendSwitch("disable-gpu-sandbox");
        command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        command_line->AppendSwitch("no-sandbox");
        command_line->AppendSwitch("disable-field-trial-config");

        // Check for display
        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != nullptr && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        if (gpu_enabled_)
        {
            // GPU acceleration mode
            command_line->AppendSwitchWithValue("use-gl", "egl-angle");
            command_line->AppendSwitchWithValue("use-angle", "egl");
            command_line->AppendSwitch("enable-gpu-rasterization");
            command_line->AppendSwitch("enable-zero-copy");
            command_line->AppendSwitch("ignore-gpu-blocklist");

            if (!has_display)
            {
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - NO DISPLAY, using headless");
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
            }
            DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU mode enabled");
        }
        else
        {
            // SwiftShader software rendering
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

        //DEBUG_LOG_GL("NOARGS!");
    }

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
    {
        return this;
    }

    /**
     * OnBeforeChildProcessLaunch:
     * @command_line: The command line that will be passed to the child process
     *
     * Called before spawning a child process. Passes GL-related switches
     * to subprocesses so they use the same GL implementation.
     */
    void OnBeforeChildProcessLaunch(CefRefPtr<CefCommandLine> command_line) override
    {
        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != NULL && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        /*
        if (gpu_enabled_)
        {
            DEBUG_LOG_GL("BeforeChildProcessLaunch - GPU enabled, using egl-angle");
            command_line->AppendSwitchWithValue("use-gl", "egl-angle");
            command_line->AppendSwitchWithValue("use-angle", "egl");
            command_line->AppendSwitch("enable-gpu-rasterization");
            command_line->AppendSwitch("ignore-gpu-blocklist");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
            }
        }
        else
        {
            DEBUG_LOG_GL("BeforeChildProcessLaunch - GPU disabled, using SwiftShader");
            command_line->AppendSwitchWithValue("use-gl", "swiftshader");
            command_line->AppendSwitchWithValue("use-angle", "swiftshader");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
            }
        }
        */

        command_line->AppendSwitch("disable-gpu-sandbox");
        command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        command_line->AppendSwitch("no-sandbox");

        // DEBUG_LOG_GL("OnBeforeChildProcessLaunch - %s", command_line->GetCommandLineString().ToString().c_str());
    }

private:
    gboolean gpu_enabled_;
    gint gpu_device_;
    IMPLEMENT_REFCOUNTING(CefManagerApp);
};

// ============================================================================
// CEF Handler implementations
// ============================================================================

/**
 * CefManagerRenderHandler - Handles offscreen rendering for a browser
 */
class CefManagerRenderHandler : public CefRenderHandler
{
public:
    CefManagerRenderHandler(CefManager* manager, BrowserInstance* browser, int width, int height)
        : manager_(manager), browser_(browser), width_(width), height_(height)
    {
    }

    void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override
    {
        rect.Set(0, 0, width_, height_);
    }

    void OnPaint(
        CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {
        if (!browser_ || !browser_->running)
        {
            return;
        }

        if (width != width_ || height != height_)
        {
            DEBUG_LOG("OnPaint - Size mismatch: got %dx%d, expected %dx%d",
                      width, height, width_, height_);
            return;
        }

        // Forward to manager
        manager_->on_browser_paint(browser_, buffer, width, height);
    }

private:
    CefManager* manager_;
    BrowserInstance* browser_;
    int width_;
    int height_;
    IMPLEMENT_REFCOUNTING(CefManagerRenderHandler);
};

/**
 * CefManagerLoadHandler - Handles page load events
 */
class CefManagerLoadHandler : public CefLoadHandler
{
public:
    CefManagerLoadHandler(CefManager* manager, BrowserInstance* browser)
        : manager_(manager), browser_(browser)
    {
    }

    void OnLoadEnd(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int httpStatusCode) override
    {
        if (frame->IsMain())
        {
            DEBUG_LOG_CEF("Page loaded (HTTP %d)", httpStatusCode);
            browser_->page_loaded = TRUE;
            manager_->on_browser_load_end(browser_, httpStatusCode);
        }
    }

    void OnLoadError(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        ErrorCode errorCode,
        const CefString& errorText,
        const CefString& failedUrl) override
    {
        if (frame->IsMain())
        {
            manager_->on_browser_load_error(
                browser_,
                errorCode,
                errorText.ToString(),
                failedUrl.ToString()
            );
        }
    }

private:
    CefManager* manager_;
    BrowserInstance* browser_;
    IMPLEMENT_REFCOUNTING(CefManagerLoadHandler);
};

/**
 * CefManagerLifeSpanHandler - Handles browser lifecycle events
 */
class CefManagerLifeSpanHandler : public CefLifeSpanHandler
{
public:
    CefManagerLifeSpanHandler(CefManager* manager, BrowserInstance* browser)
        : manager_(manager), browser_(browser)
    {
    }

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        CEF_REQUIRE_UI_THREAD();
        DEBUG_LOG_CEF("OnAfterCreated - browser=%p", browser.get());
        manager_->on_browser_created(browser_, browser);
    }

private:
    CefManager* manager_;
    BrowserInstance* browser_;
    IMPLEMENT_REFCOUNTING(CefManagerLifeSpanHandler);
};

/**
 * CefManagerClient - Main CEF client for a browser instance
 */
class CefManagerClient : public CefClient
{
public:
    CefManagerClient(
        CefRefPtr<CefRenderHandler> render_handler,
        CefRefPtr<CefLoadHandler> load_handler,
        CefRefPtr<CefLifeSpanHandler> lifespan_handler)
        : render_handler_(render_handler),
          load_handler_(load_handler),
          lifespan_handler_(lifespan_handler)
    {
    }

    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return render_handler_; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return load_handler_; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return lifespan_handler_; }

private:
    CefRefPtr<CefRenderHandler> render_handler_;
    CefRefPtr<CefLoadHandler> load_handler_;
    CefRefPtr<CefLifeSpanHandler> lifespan_handler_;
    IMPLEMENT_REFCOUNTING(CefManagerClient);
};

// ============================================================================
// CefManager Implementation
// ============================================================================

CefManager* CefManager::get()
{
    // Thread-safe static initialization (C++11 magic statics)
    g_mutex_lock(&init_mutex_);
    if (!instance_)
    {
        instance_ = new CefManager();
        if (!instance_->initialized_)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }
    g_mutex_unlock(&init_mutex_);
    return instance_;
}

CefManager::CefManager()
    : initialized_(FALSE),
      message_loop_id_(0),
      running_(FALSE),
      gpu_enabled_(FALSE),
      gpu_user_specified_(FALSE),
      gpu_device_(-1)
{
    g_mutex_init(&browsers_mutex_);

    if (!initialize_cef())
    {
        DEBUG_LOG_CEF("CefManager initialization FAILED");
        return;
    }

    initialized_ = TRUE;
    start_message_loop();
}

CefManager::~CefManager()
{
    // Stop message loop
    stop_message_loop();

    // Close all browsers
    g_mutex_lock(&browsers_mutex_);
    for (auto& pair : browser_clients_)
    {
        if (pair.first && pair.first->cef_browser)
        {
            pair.first->cef_browser->GetHost()->CloseBrowser(TRUE);
            pair.first->cef_browser = nullptr;
        }
    }
    browser_clients_.clear();
    g_mutex_unlock(&browsers_mutex_);

    // Shutdown CEF
    if (initialized_)
    {
        CefShutdown();
        DEBUG_LOG_CEF("CEF shutdown complete");
    }

    g_mutex_clear(&browsers_mutex_);
}

gboolean CefManager::initialize_cef()
{
    DEBUG_LOG_CEF("=== Initializing CEF ===");

    // Determine GPU configuration
    GpuConfig* gpu_cfg = gpu_config_new();
    if (gpu_is_available())
    {
        gpu_config_detect(gpu_cfg);
        gpu_enabled_ = gpu_cfg->enabled;
        gpu_device_ = gpu_cfg->device_index;
    }
    else
    {
        gpu_enabled_ = FALSE;
    }
    gpu_config_free(gpu_cfg);

    DEBUG_LOG_CEF("GPU config: enabled=%d, device=%d", gpu_enabled_, gpu_device_);

    // Log environment
    // DEBUG_LOG_CEF("=== Environment Variables ===");
    // DEBUG_LOG_CEF("XDG_RUNTIME_DIR: %s", g_getenv("XDG_RUNTIME_DIR") ?: "(not set)");
    // DEBUG_LOG_CEF("DBUS_SESSION_BUS_ADDRESS: %s", g_getenv("DBUS_SESSION_BUS_ADDRESS") ?: "(not set)");
    // DEBUG_LOG_CEF("HOME: %s", g_getenv("HOME") ?: "(not set)");
    // DEBUG_LOG_CEF("XAUTHORITY: %s", g_getenv("XAUTHORITY") ?: "(not set)");

    // Create CEF app
    CefMainArgs main_args;
    cef_app_ = new CefManagerApp(gpu_enabled_, gpu_device_);

    // CEF settings
    CefSettings settings;
    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_INFO;
    settings.multi_threaded_message_loop = FALSE;

    if (!gpu_enabled_)
    {
        settings.chrome_runtime = FALSE;
    }

    // Find subprocess binary
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
            DEBUG_LOG_CEF("Found subprocess: %s", found_subprocess);
            break;
        }
    }

    if (!found_subprocess)
    {
        g_free(home_subprocess);
        g_critical("chromiumsrc: subprocess binary 'chromiumsrc-subprocess' not found");
        return FALSE;
    }

    CefString(&settings.browser_subprocess_path) = found_subprocess;
    g_free(home_subprocess);

    // Create cache directory
    gchar* cache_dir = g_strdup_printf("/tmp/chromiumsrc-%d", getpid());
    g_mkdir_with_parents(cache_dir, 0700);
    CefString(&settings.root_cache_path) = cache_dir;
    CefString(&settings.cache_path) = cache_dir;
    g_free(cache_dir);

    // Find CEF resources
    const gchar* search_paths[] = {
        g_getenv("CHROMIUMSRC_RESOURCES_PATH") ? : "skip",
        g_getenv("GST_PLUGIN_PATH") ? g_strdup_printf("%s/gstreamer-1.0", g_getenv("GST_PLUGIN_PATH")) : "skip",
        home_dir ? g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins", home_dir) : "skip",
        "/usr/local/lib/gstreamer-1.0",
        "/usr/lib/gstreamer-1.0",
        "/usr/lib/x86_64-linux-gnu/gstreamer-1.0",
        nullptr
    };

    for (int i = 0; search_paths[i] != nullptr; i++)
    {
        if (g_strcmp0(search_paths[i], "skip") == 0) continue;

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
    if (!CefInitialize(main_args, settings, cef_app_, nullptr))
    {
        DEBUG_LOG_CEF("CefInitialize FAILED");
        return FALSE;
    }

    DEBUG_LOG_CEF("CEF initialized successfully");
    return TRUE;
}

void CefManager::start_message_loop()
{
    running_ = TRUE;
    // Interval based on framerate: 1000ms / fps, with minimum of 8ms
    guint interval_ms = MAX(8, 1000 / 60); // Default to 60 FPS
    message_loop_id_ = g_timeout_add(interval_ms, message_loop_callback, this);
    DEBUG_LOG_CEF("Message loop started (id=%u, interval=%ums)", message_loop_id_, interval_ms);
}

void CefManager::stop_message_loop()
{
    running_ = FALSE;
    if (message_loop_id_)
    {
        g_source_remove(message_loop_id_);
        message_loop_id_ = 0;
        DEBUG_LOG_CEF("Message loop stopped");
    }
}

gboolean CefManager::message_loop_callback(gpointer data)
{
    CefManager* self = static_cast<CefManager*>(data);

    if (!self->running_)
    {
        return G_SOURCE_REMOVE;
    }

    // Pump CEF message loop
    CefDoMessageLoopWork();

    // Invalidate all browser views to trigger repaints
    g_mutex_lock(&self->browsers_mutex_);
    for (auto& pair : self->browser_clients_)
    {
        BrowserInstance* browser = pair.first;
        if (browser && browser->running && browser->page_loaded && browser->cef_browser)
        {
            browser->cef_browser->GetHost()->Invalidate(PET_VIEW);
        }
    }
    g_mutex_unlock(&self->browsers_mutex_);

    return G_SOURCE_CONTINUE;
}

BrowserInstance* CefManager::create_browser(
    const gchar* url,
    gint width,
    gint height,
    gint fps,
    BrowserCallbacks* callbacks)
{
    if (!initialized_)
    {
        DEBUG_LOG_CEF("create_browser - CEF not initialized");
        return nullptr;
    }

    // Pump message loop a few times to ensure CEF is ready
    // 1000*1000 = 1s
    for (int i = 0; i < 1000; i++)
    {
        CefDoMessageLoopWork();
        g_usleep(1000);
    }

    // Create browser instance
    const auto browser = new BrowserInstance();
    browser->width = width;
    browser->height = height;
    browser->fps = fps;
    browser->page_loaded = FALSE;
    browser->running = TRUE;
    browser->callbacks = *callbacks;

    // Create handlers
    CefRefPtr<CefManagerRenderHandler> render_handler =
        new CefManagerRenderHandler(this, browser, width, height);
    CefRefPtr<CefManagerLoadHandler> load_handler =
        new CefManagerLoadHandler(this, browser);
    CefRefPtr<CefManagerLifeSpanHandler> lifespan_handler =
        new CefManagerLifeSpanHandler(this, browser);

    // Create client
    CefRefPtr<CefManagerClient> client =
        new CefManagerClient(render_handler, load_handler, lifespan_handler);

    // Track browser
    g_mutex_lock(&browsers_mutex_);
    browser_clients_[browser] = client;
    g_mutex_unlock(&browsers_mutex_);

    // Configure windowless rendering
    CefWindowInfo window_info;
    window_info.SetAsWindowless(0);

    // Configure browser settings
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = fps;

    CefString cef_url(url);

    DEBUG_LOG_CEF("CreateBrowser - url=%s, width=%d, height=%d, fps=%d",
                  url, width, height, fps);

    // Create browser asynchronously
    if (!CefBrowserHost::CreateBrowser(
        window_info, client, cef_url, browser_settings, nullptr, nullptr))
    {
        DEBUG_LOG_CEF("CreateBrowser FAILED");
        g_mutex_lock(&browsers_mutex_);
        browser_clients_.erase(browser);
        g_mutex_unlock(&browsers_mutex_);
        delete browser;
        return nullptr;
    }

    DEBUG_LOG_CEF("Browser creation initiated");
    return browser;
}

void CefManager::destroy_browser(BrowserInstance* browser)
{
    if (!browser) return;

    DEBUG_LOG_CEF("Destroying browser...");

    browser->running = FALSE;

    // Remove from tracking
    g_mutex_lock(&browsers_mutex_);
    browser_clients_.erase(browser);
    g_mutex_unlock(&browsers_mutex_);

    // Close CEF browser
    if (browser->cef_browser)
    {
        browser->cef_browser->GetHost()->CloseBrowser(TRUE);
        browser->cef_browser = nullptr;
    }

    delete browser;
    DEBUG_LOG_CEF("Browser destroyed");
}

CefBrowser* CefManager::get_browser_handle(BrowserInstance* browser)
{
    return browser ? browser->cef_browser.get() : nullptr;
}

gboolean CefManager::is_page_loaded(BrowserInstance* browser)
{
    return browser ? browser->page_loaded : FALSE;
}

void CefManager::on_browser_created(BrowserInstance* browser, CefRefPtr<CefBrowser> cef_browser)
{
    if (browser)
    {
        browser->cef_browser = cef_browser;
        DEBUG_LOG_CEF("Browser created and stored");
    }
}

void CefManager::on_browser_paint(
    BrowserInstance* browser,
    const void* buffer,
    int width,
    int height)
{
    if (browser && browser->callbacks.on_paint)
    {
        browser->callbacks.on_paint(browser->callbacks.user_data, buffer, width, height);
    }
}

void CefManager::on_browser_load_end(BrowserInstance* browser, int http_status_code)
{
    if (browser && browser->callbacks.on_load_end)
    {
        browser->callbacks.on_load_end(browser->callbacks.user_data, http_status_code);
    }
}

void CefManager::on_browser_load_error(
    BrowserInstance* browser,
    int error_code,
    const std::string& error_text,
    const std::string& failed_url)
{
    DEBUG_LOG_CEF("Load error: %s (%d) - %s",
                  error_text.c_str(), error_code, failed_url.c_str());
}

// ============================================================================
// C API wrapper functions
// ============================================================================

extern "C" {
gpointer cef_manager_get(void)
{
    return CefManager::get();
}

BrowserInstance* cef_manager_create_browser(
    gpointer manager,
    const gchar* url,
    gint width,
    gint height,
    gint fps,
    BrowserCallbacks* callbacks)
{
    CefManager* m = static_cast<CefManager*>(manager);
    return m ? m->create_browser(url, width, height, fps, callbacks) : nullptr;
}

void cef_manager_destroy_browser(gpointer manager, BrowserInstance* browser)
{
    CefManager* m = static_cast<CefManager*>(manager);
    if (m) m->destroy_browser(browser);
}

gpointer cef_manager_get_browser_handle(BrowserInstance* browser)
{
    return browser ? browser->cef_browser.get() : nullptr;
}

gboolean cef_manager_is_page_loaded(BrowserInstance* browser)
{
    return CefManager::get() ? CefManager::get()->is_page_loaded(browser) : FALSE;
}
}
