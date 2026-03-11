#ifndef __CEF_MANAGER_H__
#define __CEF_MANAGER_H__

#include <glib.h>
#include <include/cef_app.h>
#include <include/cef_client.h>

G_BEGIN_DECLS

/**
 * cef_manager_configure:
 * @disable_gpu: TRUE to force disable GPU, FALSE to use auto-detection
 * @gpu_user_specified: TRUE if user explicitly set GPU mode
 *
 * Configures GPU settings before CEF initialization.
 * Must be called before cef_manager_get().
 * If @gpu_user_specified is TRUE, the @disable_gpu value is used.
 * If @gpu_user_specified is FALSE, GPU is auto-detected.
 */
void cef_manager_configure(gboolean disable_gpu, gboolean gpu_user_specified);

/**
 * Gets the singleton CefManager instance.
 * Initializes CEF on first call.
 * Thread-safe.
 *
 * Returns: The singleton CefManager instance, or NULL on failure
 */
gpointer cef_manager_get(void);

/**
 * Returns: TRUE if GPU is disabled, FALSE otherwise
 */
gboolean cef_manager_is_gpu_disabled(void);

/**
 * Returns the GPU device index, or -1 if not set.
 */
gint cef_manager_get_gpu_device(void);

G_END_DECLS

/**
 * CefManager - C++ singleton class for managing CEF initialization
 *
 * This class provides:
 * - One-time CEF initialization (thread-safe)
 * - GPU configuration management
 * - CefApp for command line processing
 *
 * Browser instances are managed separately via cef_browser_client.h
 *
 * Usage:
 *   CefManager::configure(disable_gpu, gpu_user_specified);
 *   CefManager* manager = CefManager::get();
 *   // Use cef_browser_create() to create browsers
 */
class CefManager {
public:
    /**
     * configure:
     * @disable_gpu: TRUE to force disable GPU, FALSE to use auto-detection
     * @gpu_user_specified: TRUE if user explicitly set GPU mode
     *
     * Configures GPU settings before CEF initialization.
     * Must be called before get().
     */
    static void configure(gboolean disable_gpu, gboolean gpu_user_specified);

    /**
     * get:
     *
     * Returns the singleton CefManager instance.
     * Initializes CEF on first call.
     * Thread-safe via static initialization.
     *
     * Returns: The singleton instance, or nullptr on failure
     */
    static CefManager* get();

    /**
     * is_gpu_disabled:
     *
     * Returns whether GPU acceleration is disabled.
     */
    gboolean is_gpu_disabled() const { return is_gpu_disabled_; }

    /**
     * get_gpu_device:
     *
     * Returns the GPU device index, or -1 if not set.
     */
    gint get_gpu_device() const { return gpu_device_; }

private:
    /**
     * CefManager constructor - private for singleton pattern
     *
     * Initializes CEF.
     */
    CefManager();

    /**
     * CefManager destructor - private for singleton pattern
     *
     * Shuts down CEF and cleans up resources.
     */
    ~CefManager();

    /**
     * initialize_cef:
     *
     * Performs one-time CEF initialization.
     * Called once from constructor.
     *
     * Returns: TRUE on success, FALSE on failure
     */
    gboolean initialize_cef();

    // Singleton instance
    static CefManager* instance_;
    static GMutex init_mutex_;

    // Pre-initialization configuration
    static gboolean config_disable_gpu_;
    static gboolean config_gpu_user_specified_;

    // CEF state
    gboolean initialized_;
    CefRefPtr<CefApp> cef_app_;

    // GPU configuration
    gboolean is_gpu_disabled_;
    gboolean gpu_user_specified_;
    gint gpu_device_;

    // Prevent copying
    CefManager(const CefManager&) = delete;
    CefManager& operator=(const CefManager&) = delete;
};

#endif
