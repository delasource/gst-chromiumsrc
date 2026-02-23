#ifndef __CEF_RENDER_HANDLER_H__
#define __CEF_RENDER_HANDLER_H__

#include "gstchromiumsrc.h"

#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

G_BEGIN_DECLS

gboolean cef_browser_start(GstChromiumSrc *src, const gchar *url, gint width, gint height);
void cef_browser_stop(GstChromiumSrc *src);

G_END_DECLS

#endif
