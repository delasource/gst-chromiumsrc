/**
 * subprocess_main.cpp - CEF Subprocess Binary for Chromium Multi-Process Architecture
 *
 * Purpose:
 *   CEF uses a multi-process architecture where the main browser process spawns
 *   separate subprocesses for rendering, GPU acceleration, and utility tasks.
 *   This binary serves as the entry point for all those subprocesses.
 *
 * Architecture:
 *   - Main process: gstchromiumsrc plugin (loads CEF, creates browser)
 *   - Subprocesses: This binary (handles renderer, GPU, utility processes)
 *
 * How it works:
 *   1. Main process sets browser_subprocess_path to point to this binary
 *   2. When CEF needs a subprocess, it executes this binary with special flags
 *   3. CefExecuteProcess() detects it's running as subprocess and handles it
 *   4. Returns exit_code >= 0 if this is a subprocess, < 0 if main process
 *
 * Build:
 *   Built as a standalone executable linked against CEF libraries
 *   Installed alongside the GStreamer plugin
 *
 * References:
 *   - https://bitbucket.org/chromiumembedded/cef/wiki/GeneralUsage.md
 *   - cef_render_handler.cpp:initialize_cef() configures the subprocess path
 */

#include <include/cef_app.h>
#include <include/cef_command_line.h>
#include <glib.h>
#include <string>
#include <cstring>

/**
 * CefSubprocessApp - CEF application handler for subprocess execution
 *
 * Implements CefApp and CefBrowserProcessHandler to provide command line
 * processing for subprocess instances. This ensures that subprocesses
 * receive the same Chromium flags as the main browser process.
 *
 * Key responsibilities:
 *   - Apply headless rendering flags when no DISPLAY is available
 *   - Disable sandboxing and GPU sandbox for containerized environments
 *   - Disable unnecessary features (extensions, sync, background networking)
 */
class CefSubprocessApp : public CefApp
{
public:
    CefSubprocessApp() : process_type_("unknown")
    {
    }

    std::string GetProcessType() const { return process_type_; }

    /**
         * OnBeforeCommandLineProcessing:
         * @process_type: Type of subprocess (renderer, gpu-process, utility, etc.)
         * @command_line: The command line to modify before subprocess starts
         *
         * Configures Chromium command line switches for the subprocess.
         * Called by CEF before spawning each subprocess type.
         *
         * Applied flags:
         *   - Security: disable-gpu-sandbox, disable-seccomp-filter-sandbox, no-sandbox
         *   - Performance: disable-extensions, disable-sync, disable-background-networking
         *   - Headless: ozone-platform=headless, headless=new (when no DISPLAY)
         */
    void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) override
    {
        process_type_ = process_type.ToString();
        g_print("[%s] =====================\n", process_type_.c_str());
        g_print("[%s] CEF Subprocess Binary - Build Time: %s %s\n", process_type_.c_str(), __DATE__, __TIME__);
        g_print("[%s] =====================\n", process_type_.c_str());

        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != nullptr && g_strcmp0(display, "NULL") != 0 && strlen(display) > 0;

        // Disable browser extensions - not needed for headless rendering
        // command_line->AppendSwitch("disable-extensions");
        // Disable Chrome sync services - not needed for this use case
        // command_line->AppendSwitch("disable-sync");
        // Disable background network activity to reduce resource usage
        // command_line->AppendSwitch("disable-background-networking");
        // Skip first-run wizard and welcome pages
        // command_line->AppendSwitch("no-first-run");
        // Disable GPU sandbox - needed for containerized/privileged environments
        // command_line->AppendSwitch("disable-gpu-sandbox");
        // Disable seccomp-bpf filter sandbox - needed for container compatibility
        // command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        // Disable all sandboxing - required for running in Docker/containers
        // command_line->AppendSwitch("no-sandbox");
        // Disable field trial experiments for deterministic behavior
        // command_line->AppendSwitch("disable-field-trial-config");
        // Enable verbose logging for debugging
        command_line->AppendSwitchWithValue("log-severity", "verbose");

        if (!has_display)
        {
            g_print("[%s] No valid DISPLAY, using headless mode\n", process_type_.c_str());
            // Use Ozone headless platform for rendering without a display server
            command_line->AppendSwitchWithValue("ozone-platform", "headless");
            // Enable new headless mode (Chrome's modern headless implementation)
            command_line->AppendSwitchWithValue("headless", "new");

            // Vulkan ANGLE backend for headless GPU (matches browser process flags)
            command_line->AppendSwitchWithValue("use-angle", "vulkan");
            command_line->AppendSwitchWithValue("enable-features", "Vulkan");
            command_line->AppendSwitch("disable-vulkan-surface");
        }

        g_print("[%s] Final command line: %s\n", process_type_.c_str(),
                command_line->GetCommandLineString().ToString().c_str());
    }

    IMPLEMENT_REFCOUNTING(CefSubprocessApp);

private:
    std::string process_type_;
};

/**
 * main - Entry point for CEF subprocess execution
 * @argc: Argument count from command line
 * @argv: Argument vector from command line
 *
 * This function serves a dual purpose:
 *   1. When executed by CEF as a subprocess, CefExecuteProcess() handles
 *      the subprocess logic and returns exit_code >= 0
 *   2. When run directly (shouldn't happen in normal operation), returns 0
 *
 * CEF passes special command-line arguments when spawning subprocesses,
 * which CefExecuteProcess() uses to determine the process type and role.
 *
 * Returns: Exit code from subprocess execution, or 0 if main process
 */
int main(int argc, char* argv[])
{
    // Sleep for 1ms, so CEF main process can think a moment
    g_usleep(1000);

    g_print("[main] subprocess_main invoked with %d args:\n", argc);
    for (int i = 0; i < argc; i++)
    {
        g_print("     %s\n", argv[i]);
    }

    CefMainArgs main_args(argc, argv);
    const CefRefPtr app = new CefSubprocessApp();

    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0)
    {
        g_print("[%s] CefExecuteProcess returned: %d\n",
                app->GetProcessType().c_str(), exit_code);
    }

    g_print("[%s] EOF", app->GetProcessType().c_str());

    return exit_code;
}
