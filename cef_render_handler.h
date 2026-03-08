#ifndef __CEF_RENDER_HANDLER_H__
#define __CEF_RENDER_HANDLER_H__

#include "gstchromiumsrc.h"

G_BEGIN_DECLS

/**
 * cef_browser_start:
 * @src: The GstChromiumSrc instance
 * @url: The URL to load in the browser
 * @width: Width of the rendering surface in pixels
 * @height: Height of the rendering surface in pixels
 *
 * Creates and starts a CEF browser instance for offscreen rendering.
 * Uses the global CefManager singleton for browser management.
 *
 * Invoked by gst_chromium_src_start() during the READY_TO_PAUSED
 * state transition.
 *
 * Returns: TRUE on success, FALSE on failure
 */
gboolean cef_browser_start(GstChromiumSrc *src, const gchar *url, gint width, gint height);

/**
 * cef_browser_stop:
 * @src: The GstChromiumSrc instance
 *
 * Stops and cleans up the CEF browser instance.
 *
 * Invoked by gst_chromium_src_stop() during the PAUSED_TO_READY
 * state transition.
 */
void cef_browser_stop(GstChromiumSrc *src);

G_END_DECLS

#endif
