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
gboolean CefManager::config_disable_gpu_ = FALSE;
gboolean CefManager::config_gpu_user_specified_ = FALSE;

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
    CefManagerApp(gboolean is_gpu_disabled, gint gpu_device)
        : is_gpu_disabled_(is_gpu_disabled), gpu_device_(gpu_device)
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
        //command_line->AppendSwitchWithValue("log-severity", "verbose");

        // Sandbox settings
        // These are already present because of CefSettings.no_sandbox
        // command_line->AppendSwitch("disable-gpu-sandbox");
        // command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        // command_line->AppendSwitch("no-sandbox");
        command_line->AppendSwitch("disable-field-trial-config");

        // Check for display
        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != nullptr && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        if (!is_gpu_disabled_)
        {
            // GPU acceleration mode - let Chromium auto-detect best GL implementation
            // Don't force specific GL/ANGLE backends as they may not work on all systems
            command_line->AppendSwitch("enable-zero-copy");
            command_line->AppendSwitch("ignore-gpu-blocklist");

            if (!has_display)
            {
                DEBUG_LOG_GL("OnBeforeCommandLineProcessing - NO DISPLAY, using headless + Vulkan ANGLE");
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");

                // Force Vulkan backend for ANGLE - bypasses EGL's X11 dependency.
                // Without this, ANGLE tries EGL → X11 → "Could not open the default X display"
                // See: chromium.googlesource.com/chromium/src/+/HEAD/docs/gpu/using-gpu-hardware-in-headless-chrome.md
                command_line->AppendSwitchWithValue("use-angle", "vulkan");
                command_line->AppendSwitchWithValue("enable-features", "Vulkan");
                command_line->AppendSwitch("disable-vulkan-surface");
            }
            DEBUG_LOG_GL("OnBeforeCommandLineProcessing - GPU mode enabled (display=%s)",
                         has_display ? "yes" : "no");
        }
        else
        {
            // SwiftShader software rendering
            command_line->AppendSwitchWithValue("use-gl", "swiftshader");
            command_line->AppendSwitchWithValue("use-angle", "swiftshader");
            command_line->AppendSwitch("in-process-gpu");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
            }
            DEBUG_LOG_GL("OnBeforeCommandLineProcessing - SwiftShader software rendering");
        }

        DEBUG_LOG_GL("OnBeforeCommandLineProcessing - final cmdline: %s",
                     command_line->GetCommandLineString().ToString().c_str());
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
        // These are already present because of CefSettings.no_sandbox
        // command_line->AppendSwitch("disable-gpu-sandbox");
        // command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        // command_line->AppendSwitch("no-sandbox");
        // command_line->AppendSwitchWithValue("log-severity", "verbose");

        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != NULL && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        if (!is_gpu_disabled_)
        {
            // GPU enabled - let Chromium auto-detect best GL implementation
            command_line->AppendSwitch("ignore-gpu-blocklist");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");

                // Vulkan ANGLE backend for headless GPU (same as browser process)
                command_line->AppendSwitchWithValue("use-angle", "vulkan");
                command_line->AppendSwitchWithValue("enable-features", "Vulkan");
                command_line->AppendSwitch("disable-vulkan-surface");
            }
        }
        else
        {
            //DEBUG_LOG_GL("BeforeChildProcessLaunch - GPU disabled, using SwiftShader");
            command_line->AppendSwitchWithValue("use-gl", "swiftshader");
            command_line->AppendSwitchWithValue("use-angle", "swiftshader");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
            }
        }

        DEBUG_LOG_GL("OnBeforeChildProcessLaunch - gpu=%s, display=%s, cmdline: %s",
                     is_gpu_disabled_ ? "off" : "on",
                     has_display ? "yes" : "no",
                     command_line->GetCommandLineString().ToString().c_str());
    }

private:
    gboolean is_gpu_disabled_;
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

void CefManager::configure(gboolean disable_gpu, gboolean gpu_user_specified)
{
    config_disable_gpu_ = disable_gpu;
    config_gpu_user_specified_ = gpu_user_specified;
}

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
      running_(FALSE),
      is_gpu_disabled_(TRUE),
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
}

CefManager::~CefManager()
{
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
    else
    {
        DEBUG_LOG_CEF("Deconstructed without CEF shutdown");
    }

    g_mutex_clear(&browsers_mutex_);
}

gboolean CefManager::initialize_cef()
{
    DEBUG_LOG_CEF("=== Initializing CEF ===");

    // Determine GPU configuration
    if (config_gpu_user_specified_)
    {
        // User explicitly set GPU mode
        is_gpu_disabled_ = config_disable_gpu_;
        gpu_device_ = -1;
        DEBUG_LOG_CEF("GPU user-specified: disabled=%d", is_gpu_disabled_);
    }
    else
    {
        // Auto-detect GPU
        GpuConfig* gpu_cfg = gpu_config_new();
        if (gpu_is_available())
        {
            gpu_config_detect(gpu_cfg);
            is_gpu_disabled_ = !gpu_cfg->enabled;
            gpu_device_ = gpu_cfg->device_index;
        }
        else
        {
            is_gpu_disabled_ = TRUE;
        }
        gpu_config_free(gpu_cfg);
    }

    DEBUG_LOG_CEF("GPU config: disabled=%d, device=%d", is_gpu_disabled_, gpu_device_);

    // Log environment for debugging headless/GPU issues
    const gchar* display_env = g_getenv("DISPLAY");
    const gchar* wayland_env = g_getenv("WAYLAND_DISPLAY");
    const gchar* xdg_rt = g_getenv("XDG_RUNTIME_DIR");
    DEBUG_LOG_CEF("Environment: DISPLAY=%s, WAYLAND_DISPLAY=%s, XDG_RUNTIME_DIR=%s",
                  display_env ? display_env : "(not set)",
                  wayland_env ? wayland_env : "(not set)",
                  xdg_rt ? xdg_rt : "(not set)");
    // DEBUG_LOG_CEF("DBUS_SESSION_BUS_ADDRESS: %s", g_getenv("DBUS_SESSION_BUS_ADDRESS") ?: "(not set)");
    // DEBUG_LOG_CEF("HOME: %s", g_getenv("HOME") ?: "(not set)");
    // DEBUG_LOG_CEF("XAUTHORITY: %s", g_getenv("XAUTHORITY") ?: "(not set)");

    // Create CEF app
    CefMainArgs main_args;
    cef_app_ = new CefManagerApp(is_gpu_disabled_, gpu_device_);

    // CEF settings
    CefSettings settings;
    settings.no_sandbox = TRUE;
    settings.windowless_rendering_enabled = TRUE;
    settings.log_severity = LOGSEVERITY_INFO;
    settings.multi_threaded_message_loop = TRUE;
    CefString(&settings.log_file) = "/tmp/chromiumsrc_cef.log";

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

    // Create browser instance
    const auto browser = new BrowserInstance();
    browser->width = width;
    browser->height = height;
    browser->fps = fps;
    browser->page_loaded = FALSE;
    browser->running = TRUE;
    browser->callbacks = *callbacks;

    // Create handlers
    const auto render_handler = new CefManagerRenderHandler(this, browser, width, height);
    const auto load_handler = new CefManagerLoadHandler(this, browser);
    const auto lifespan_handler = new CefManagerLifeSpanHandler(this, browser);

    // Create client
    const auto client = new CefManagerClient(render_handler, load_handler, lifespan_handler);

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

    DEBUG_LOG_CEF("CreateBrowser - url=%s, width=%d, height=%d, fps=%d", url, width, height, fps);

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
void cef_manager_configure(gboolean disable_gpu, gboolean gpu_user_specified)
{
    CefManager::configure(disable_gpu, gpu_user_specified);
}

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
