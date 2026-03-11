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
            // GPU acceleration mode
            command_line->AppendSwitchWithValue("use-gl", "egl-angle");
            command_line->AppendSwitchWithValue("use-angle", "egl");
            // command_line->AppendSwitch("enable-gpu-rasterization");
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
            command_line->AppendSwitchWithValue("use-angle", "swiftshader");
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
        // These are already present because of CefSettings.no_sandbox
        // command_line->AppendSwitch("disable-gpu-sandbox");
        // command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        // command_line->AppendSwitch("no-sandbox");
        // command_line->AppendSwitchWithValue("log-severity", "verbose");

        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != NULL && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        if (!is_gpu_disabled_)
        {
            // DEBUG_LOG_GL("BeforeChildProcessLaunch - GPU enabled, using egl-angle");
            command_line->AppendSwitchWithValue("use-gl", "egl-angle");
            command_line->AppendSwitchWithValue("use-angle", "egl");
            // command_line->AppendSwitch("enable-gpu-rasterization");
            command_line->AppendSwitch("ignore-gpu-blocklist");

            if (!has_display)
            {
                command_line->AppendSwitchWithValue("ozone-platform", "headless");
                command_line->AppendSwitchWithValue("headless", "new");
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

        // DEBUG_LOG_GL("OnBeforeChildProcessLaunch - %s", command_line->GetCommandLineString().ToString().c_str());
    }

private:
    gboolean is_gpu_disabled_;
    gint gpu_device_;
    IMPLEMENT_REFCOUNTING(CefManagerApp);
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
      is_gpu_disabled_(TRUE),
      gpu_user_specified_(FALSE),
      gpu_device_(-1)
{
    if (!initialize_cef())
    {
        DEBUG_LOG_CEF("CefManager initialization FAILED");
        return;
    }

    initialized_ = TRUE;
}

CefManager::~CefManager()
{
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

    // Log environment
    // DEBUG_LOG_CEF("=== Environment Variables ===");
    // DEBUG_LOG_CEF("XDG_RUNTIME_DIR: %s", g_getenv("XDG_RUNTIME_DIR") ?: "(not set)");
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
    // On macOS: resources are inside the framework bundle at plugins/
    // On Linux: resources are at plugins/Resources/
    std::vector<gchar*> search_paths;
    
    const gchar* env_resources = g_getenv("CHROMIUMSRC_RESOURCES_PATH");
    if (env_resources) {
        search_paths.push_back(g_strdup(env_resources));
    }
    
    const gchar* gst_plugin_path = g_getenv("GST_PLUGIN_PATH");
    if (gst_plugin_path) {
        search_paths.push_back(g_strdup_printf("%s/gstreamer-1.0", gst_plugin_path));
    }
    
    if (home_dir) {
        // macOS: framework bundle in plugins dir
        search_paths.push_back(g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins/Chromium Embedded Framework.framework", home_dir));
        // Linux: direct Resources folder
        search_paths.push_back(g_strdup_printf("%s/.local/share/gstreamer-1.0/plugins", home_dir));
    }
    
    search_paths.push_back(g_strdup("/usr/local/lib/gstreamer-1.0/Chromium Embedded Framework.framework"));
    search_paths.push_back(g_strdup("/usr/local/lib/gstreamer-1.0"));
    search_paths.push_back(g_strdup("/usr/lib/gstreamer-1.0/Chromium Embedded Framework.framework"));
    search_paths.push_back(g_strdup("/usr/lib/gstreamer-1.0"));
    search_paths.push_back(g_strdup("/usr/lib/x86_64-linux-gnu/gstreamer-1.0"));
    search_paths.push_back(nullptr);

    for (size_t i = 0; i < search_paths.size() && search_paths[i] != nullptr; i++)
    {
        gchar* base_path = search_paths[i];
        
        // Check if this is a framework bundle (macOS) - resources are directly inside
        // Check if this is a regular directory (Linux) - resources are in Resources/ subdirectory
        gchar* resources_dir = g_strdup_printf("%s/Resources", base_path);
        
        // If Resources/ doesn't exist, check if base_path itself contains icudtl.dat (framework case)
        if (!g_file_test(resources_dir, G_FILE_TEST_IS_DIR)) {
            g_free(resources_dir);
            resources_dir = g_strdup(base_path);
        }
        
        if (g_file_test(resources_dir, G_FILE_TEST_IS_DIR))
        {
            gchar* icu_file = g_strdup_printf("%s/icudtl.dat", resources_dir);
            if (g_file_test(icu_file, G_FILE_TEST_EXISTS))
            {
                CefString(&settings.resources_dir_path) = resources_dir;
                DEBUG_LOG_CEF("Found CEF resources at: %s", resources_dir);
                g_free(icu_file);
                // Free remaining paths
                for (size_t j = i + 1; j < search_paths.size() && search_paths[j] != nullptr; j++) {
                    g_free(search_paths[j]);
                }
                break;
            }
            g_free(icu_file);
        }
        g_free(resources_dir);
        g_free(base_path);
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

gboolean cef_manager_is_gpu_disabled(void)
{
    CefManager* manager = CefManager::get();
    return manager ? manager->is_gpu_disabled() : TRUE;
}

gint cef_manager_get_gpu_device(void)
{
    CefManager* manager = CefManager::get();
    return manager ? manager->get_gpu_device() : -1;
}

}
