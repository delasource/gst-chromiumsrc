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
#include "gpu_utils.h"

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
        g_print("[%s] Subprocess starting\n", process_type_.c_str());
        //g_print("[%s] Initial command line: %s\n", process_type_.c_str(), command_line->GetCommandLineString().ToString().c_str());

        command_line->AppendSwitch("disable-extensions");
        command_line->AppendSwitch("disable-sync");
        command_line->AppendSwitch("disable-background-networking");
        command_line->AppendSwitch("no-first-run");
        command_line->AppendSwitch("disable-gpu-sandbox");
        command_line->AppendSwitch("disable-seccomp-filter-sandbox");
        command_line->AppendSwitch("no-sandbox");

        const gchar* display = g_getenv("DISPLAY");
        gboolean has_display = display != NULL && 
                               g_strcmp0(display, "NULL") != 0 &&
                               strlen(display) > 0;

        /*
        gboolean should_enable_gpu = gpu_is_available();
        g_print("[%s] GPU available check: %d\n", process_type_.c_str(), should_enable_gpu);

        if (should_enable_gpu)
        {
            g_print("[%s] Disabling GPU, using software compositing\n", process_type_.c_str());
            command_line->AppendSwitch("disable-gpu");
            command_line->AppendSwitch("disable-software-rasterizer");
            command_line->AppendSwitchWithValue("num-raster-threads", "4");
        }
        else
        {
            g_print("[%s] Setting use-gl=swiftshader\n", process_type_.c_str());
            command_line->AppendSwitchWithValue("use-gl", "swiftshader");
            command_line->AppendSwitch("disable-gpu");
            command_line->AppendSwitch("in-process-gpu");
        }
        */

        if (!has_display)
        {
            g_print("[%s] No valid DISPLAY, using headless mode\n", process_type_.c_str());
            command_line->AppendSwitchWithValue("ozone-platform", "headless");
            command_line->AppendSwitchWithValue("headless", "new");
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
    CefMainArgs main_args(argc, argv);
    CefRefPtr<CefSubprocessApp> app = new CefSubprocessApp();

    g_print("=====================\n");
    g_print("CEF Subprocess Binary - Build Time: %s %s\n", __DATE__, __TIME__);
    g_print("=====================\n");

    int exit_code = CefExecuteProcess(main_args, app, nullptr);

    if (exit_code >= 0)
    {
        g_print("[%s] CefExecuteProcess returned: %d\n",
                app->GetProcessType().c_str(), exit_code);
    }

    return exit_code;
}
