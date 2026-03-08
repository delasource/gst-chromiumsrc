#ifndef __CEF_MANAGER_H__
#define __CEF_MANAGER_H__

#include <glib.h>
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <map>
#include <string>

G_BEGIN_DECLS

/**
 * BrowserInstance:
 *
 * Opaque handle representing a browser instance managed by CefManager.
 * Created by cef_manager_create_browser() and destroyed by cef_manager_destroy_browser().
 */
typedef struct _BrowserInstance BrowserInstance;

/**
 * BrowserCallbacks:
 * @on_paint: Called when a new frame is available (BGRA data)
 * @on_load_end: Called when page load completes
 * @user_data: User-provided data passed to callbacks
 *
 * Callbacks for browser instance events.
 */
typedef struct {
    void (*on_paint)(void* user_data, const void* buffer, int width, int height);
    void (*on_load_end)(void* user_data, int http_status_code);
    void* user_data;
} BrowserCallbacks;

/**
 * cef_manager_get:
 *
 * Returns the singleton CefManager instance. Initializes CEF on first call.
 * Thread-safe.
 *
 * Returns: The singleton CefManager instance, or NULL on failure
 */
gpointer cef_manager_get(void);

/**
 * cef_manager_create_browser:
 * @manager: The CefManager instance
 * @url: URL to load in the browser
 * @width: Width of the rendering surface in pixels
 * @height: Height of the rendering surface in pixels
 * @fps: Desired frame rate for rendering
 * @callbacks: Callback functions for browser events
 *
 * Creates a new browser instance for offscreen rendering.
 * The browser runs within the shared CEF message loop.
 *
 * Returns: A new BrowserInstance handle, or NULL on failure
 */
BrowserInstance* cef_manager_create_browser(
    gpointer manager,
    const gchar* url,
    gint width,
    gint height,
    gint fps,
    BrowserCallbacks* callbacks
);

/**
 * cef_manager_destroy_browser:
 * @manager: The CefManager instance
 * @browser: The browser instance to destroy
 *
 * Destroys a browser instance and releases its resources.
 * Safe to call with NULL browser.
 */
void cef_manager_destroy_browser(
    gpointer manager,
    BrowserInstance* browser
);

/**
 * cef_manager_get_browser_handle:
 * @browser: The browser instance
 *
 * Returns the underlying CefBrowser pointer for advanced operations.
 * Use with caution - the pointer is valid only while the browser is alive.
 *
 * Returns: The CefBrowser pointer, or NULL
 */
gpointer cef_manager_get_browser_handle(BrowserInstance* browser);

/**
 * cef_manager_is_page_loaded:
 * @browser: The browser instance
 *
 * Returns whether the page has finished loading.
 *
 * Returns: TRUE if page loaded, FALSE otherwise
 */
gboolean cef_manager_is_page_loaded(BrowserInstance* browser);

G_END_DECLS

/**
 * CefManager - C++ singleton class for managing CEF initialization and browsers
 *
 * This class provides:
 * - One-time CEF initialization (thread-safe)
 * - Single shared message loop for all browser instances
 * - Browser instance tracking and lifecycle management
 * - GPU configuration management
 *
 * Usage:
 *   CefManager* manager = CefManager::get();
 *   BrowserInstance* browser = manager->create_browser(url, w, h, fps, callbacks);
 *   // ... use browser ...
 *   manager->destroy_browser(browser);
 */
class CefManager {
public:
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
     * create_browser:
     * @url: URL to load
     * @width: Render width
     * @height: Render height
     * @fps: Target frame rate
     * @callbacks: Event callbacks
     *
     * Creates a new browser instance with the given configuration.
     * The browser will start loading the URL immediately.
     *
     * Returns: New BrowserInstance, or nullptr on failure
     */
    BrowserInstance* create_browser(
        const gchar* url,
        gint width,
        gint height,
        gint fps,
        BrowserCallbacks* callbacks
    );

    /**
     * destroy_browser:
     * @browser: Browser to destroy
     *
     * Closes the browser and removes it from tracking.
     * Safe to call with nullptr.
     */
    void destroy_browser(BrowserInstance* browser);

    /**
     * get_browser_handle:
     * @browser: Browser instance
     *
     * Returns the raw CefBrowser pointer.
     */
    CefBrowser* get_browser_handle(BrowserInstance* browser);

    /**
     * is_page_loaded:
     * @browser: Browser instance
     *
     * Returns whether the page finished loading.
     */
    gboolean is_page_loaded(BrowserInstance* browser);

    /**
     * is_gpu_enabled:
     *
     * Returns whether GPU acceleration is enabled.
     */
    gboolean is_gpu_enabled() const { return gpu_enabled_; }

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
     * Initializes CEF and starts the message loop.
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

    /**
     * start_message_loop:
     *
     * Starts the GLib-based message loop for CEF.
     * Uses g_timeout_add to periodically call CefDoMessageLoopWork().
     */
    void start_message_loop();

    /**
     * stop_message_loop:
     *
     * Stops the GLib message loop.
     */
    void stop_message_loop();

    /**
     * message_loop_callback:
     *
     * Static callback for GLib timeout.
     * Calls CefDoMessageLoopWork() and invalidates all browser views.
     */
    static gboolean message_loop_callback(gpointer data);

    /**
     * on_browser_created:
     * @browser: The newly created browser instance
     * @cef_browser: The CEF browser handle
     *
     * Called by CefLifeSpanHandler when a browser is created.
     * Stores the CEF browser handle in the BrowserInstance.
     */
    void on_browser_created(BrowserInstance* browser, CefRefPtr<CefBrowser> cef_browser);

    /**
     * on_browser_paint:
     * @browser: The browser instance
     * @buffer: BGRA pixel data
     * @width: Buffer width
     * @height: Buffer height
     *
     * Called by CefRenderHandler when a new frame is painted.
     * Forwards to the on_paint callback.
     */
    void on_browser_paint(BrowserInstance* browser, const void* buffer, int width, int height);

    /**
     * on_browser_load_end:
     * @browser: The browser instance
     * @http_status_code: HTTP status of the load
     *
     * Called by CefLoadHandler when page load completes.
     */
    void on_browser_load_end(BrowserInstance* browser, int http_status_code);

    /**
     * on_browser_load_error:
     * @browser: The browser instance
     * @error_code: CEF error code
     * @error_text: Error description
     * @failed_url: URL that failed to load
     *
     * Called by CefLoadHandler on load errors.
     */
    void on_browser_load_error(
        BrowserInstance* browser,
        int error_code,
        const std::string& error_text,
        const std::string& failed_url
    );

    // Friend classes for handler access
    friend class CefManagerRenderHandler;
    friend class CefManagerLoadHandler;
    friend class CefManagerLifeSpanHandler;
    friend class CefManagerClient;

    // Singleton instance
    static CefManager* instance_;
    static GMutex init_mutex_;

    // CEF state
    gboolean initialized_;
    CefRefPtr<CefApp> cef_app_;

    // Message loop
    guint message_loop_id_;
    gboolean running_;

    // GPU configuration
    gboolean gpu_enabled_;
    gboolean gpu_user_specified_;
    gint gpu_device_;

    // Browser tracking
    GMutex browsers_mutex_;
    std::map<BrowserInstance*, CefRefPtr<CefClient>> browser_clients_;

    // Prevent copying
    CefManager(const CefManager&) = delete;
    CefManager& operator=(const CefManager&) = delete;
};

#endif
