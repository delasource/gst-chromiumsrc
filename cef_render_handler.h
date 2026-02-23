#ifndef __CEF_RENDER_HANDLER_H__
#define __CEF_RENDER_HANDLER_H__

#include "gstcefsrc.h"

#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

G_BEGIN_DECLS

gboolean cef_browser_start(GstCefSrc *src, const gchar *url, gint width, gint height);
void cef_browser_stop(GstCefSrc *src);

G_END_DECLS

#endif
