#ifndef __CEF_BROWSER_CLIENT_H__
#define __CEF_BROWSER_CLIENT_H__

#include <glib.h>
#include <include/cef_app.h>
#include <include/cef_client.h>

G_BEGIN_DECLS

/**
 * BrowserInstance:
 *
 * Opaque handle representing a browser instance.
 * Created by cef_browser_create() and destroyed by cef_browser_destroy().
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
 * cef_browser_create:
 * @url: URL to load in the browser
 * @width: Width of the rendering surface in pixels
 * @height: Height of the rendering surface in pixels
 * @fps: Desired frame rate for rendering
 * @callbacks: Callback functions for browser events
 *
 * Creates a new browser instance for offscreen rendering.
 *
 * Returns: A new BrowserInstance handle, or NULL on failure
 */
BrowserInstance* cef_browser_create(
    const gchar* url,
    gint width,
    gint height,
    gint fps,
    BrowserCallbacks* callbacks
);

/**
 * cef_browser_destroy:
 * @browser: The browser instance to destroy
 *
 * Destroys a browser instance and releases its resources.
 * Safe to call with NULL browser.
 */
void cef_browser_destroy(BrowserInstance* browser);

/**
 * cef_browser_get_handle:
 * @browser: The browser instance
 *
 * Returns the underlying CefBrowser pointer for advanced operations.
 * Use with caution - the pointer is valid only while the browser is alive.
 *
 * Returns: The CefBrowser pointer, or NULL
 */
gpointer cef_browser_get_handle(BrowserInstance* browser);

/**
 * cef_browser_is_page_loaded:
 * @browser: The browser instance
 *
 * Returns whether the page has finished loading.
 *
 * Returns: TRUE if page loaded, FALSE otherwise
 */
gboolean cef_browser_is_page_loaded(BrowserInstance* browser);

G_END_DECLS

#endif
