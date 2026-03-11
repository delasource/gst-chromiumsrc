#include "cef_gst_bridge.h"
#include "cef_browser_client.h"
#include "cef_manager.h"
#include "debug_utils.h"

#include <glib.h>

// ============================================================================
// Callback wrappers for GstChromiumSrc integration
// ============================================================================

/**
 * on_paint_callback:
 * @user_data: The GstChromiumSrc instance
 * @buffer: BGRA pixel data
 * @width: Buffer width
 * @height: Buffer height
 *
 * Called when a new frame is available from the browser.
 * Copies the frame data to the GStreamer element's frame buffer.
 */
static void on_paint_callback(void* user_data, const void* buffer, int width, int height) {
    GstChromiumSrc* src = static_cast<GstChromiumSrc*>(user_data);

    if (!src || !src->running || !src->frame_buffer) {
        return;
    }

    g_mutex_lock(&src->frame_mutex);

    // Verify size matches expected frame size
    gsize expected_size = static_cast<gsize>(width * height * 4);
    if (expected_size != src->frame_size) {
        DEBUG_LOG("on_paint - Size mismatch: got %zu, expected %zu",
                  expected_size, src->frame_size);
        g_mutex_unlock(&src->frame_mutex);
        return;
    }

    // Copy frame data
    memcpy(src->frame_buffer, buffer, src->frame_size);
    src->frame_ready = TRUE;
    g_cond_signal(&src->frame_cond);

    g_mutex_unlock(&src->frame_mutex);
}

/**
 * on_load_end_callback:
 * @user_data: The GstChromiumSrc instance
 * @http_status_code: HTTP status of the load
 *
 * Called when page load completes.
 */
static void on_load_end_callback(void* user_data, int http_status_code) {
    GstChromiumSrc* src = static_cast<GstChromiumSrc*>(user_data);

    if (src) {
        src->page_loaded = TRUE;
        DEBUG_LOG_CEF("Page load complete (HTTP %d)", http_status_code);
    }
}

// ============================================================================
// Public API - Wraps CefManager and CefBrowserClient for GStreamer
// ============================================================================

extern "C" {

/**
 * cef_browser_start:
 * @src: The GstChromiumSrc instance
 * @url: The URL to load in the browser
 * @width: Width of the rendering surface in pixels
 * @height: Height of the rendering surface in pixels
 *
 * Creates and starts a CEF browser instance for offscreen rendering.
 * Uses the global CefManager singleton, allowing multiple browsers
 * to share a single CEF initialization.
 *
 * Returns: TRUE on success, FALSE on failure
 */
gboolean cef_browser_start(GstChromiumSrc* src, const gchar* url, gint width, gint height) {
    DEBUG_LOG_CEF("cef_browser_start - url=%s, width=%d, height=%d", url, width, height);

    // Configure GPU settings before CEF initialization
    cef_manager_configure(!src->gpu_enabled, src->gpu_user_specified);

    // Get the singleton manager (initializes CEF if needed)
    CefManager* manager = static_cast<CefManager*>(cef_manager_get());
    if (!manager) {
        DEBUG_LOG_CEF("cef_browser_start - Failed to get CefManager");
        return FALSE;
    }

    // Set up callbacks
    BrowserCallbacks callbacks = {};
    callbacks.on_paint = on_paint_callback;
    callbacks.on_load_end = on_load_end_callback;
    callbacks.user_data = src;

    // Create browser through browser client
    BrowserInstance* browser = cef_browser_create(
        url,
        width,
        height,
        src->fps_num,
        &callbacks
    );

    if (!browser) {
        DEBUG_LOG_CEF("cef_browser_start - Failed to create browser");
        return FALSE;
    }

    // Store browser instance in src
    src->cef_browser = static_cast<gpointer>(browser);

    // Copy GPU settings from manager
    src->gpu_enabled = !manager->is_gpu_disabled();
    src->gpu_device = manager->get_gpu_device();

    DEBUG_LOG_CEF("cef_browser_start - Browser created successfully");
    return TRUE;
}

/**
 * cef_browser_stop:
 * @src: The GstChromiumSrc instance
 *
 * Stops and cleans up the CEF browser instance.
 * The CefManager singleton remains alive for other browsers.
 */
void cef_browser_stop(GstChromiumSrc* src) {
    DEBUG_LOG_CEF("cef_browser_stop");

    if (!src) {
        return;
    }

    // Destroy browser through browser client
    if (src->cef_browser) {
        cef_browser_destroy(static_cast<BrowserInstance*>(src->cef_browser));
        src->cef_browser = NULL;
    }

    // Note: We don't shut down CefManager here - it remains alive for other browsers
    DEBUG_LOG_CEF("cef_browser_stop - Browser stopped");
}

}
