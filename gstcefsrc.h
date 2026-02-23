#ifndef __GST_CEF_SRC_H__
#define __GST_CEF_SRC_H__

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

G_BEGIN_DECLS

#define GST_TYPE_CEF_SRC (gst_cef_src_get_type())
#define GST_CEF_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_CEF_SRC, GstCefSrc))
#define GST_CEF_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_CEF_SRC, GstCefSrcClass))
#define GST_IS_CEF_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_CEF_SRC))
#define GST_IS_CEF_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_CEF_SRC))

typedef struct _GstCefSrc GstCefSrc;
typedef struct _GstCefSrcClass GstCefSrcClass;

struct _GstCefSrc {
    GstBin parent;

    GstAppSrc *appsrc;
    GstPad *ghostpad;

    gchar *url;
    gint width;
    gint height;
    gint fps_num;
    gint fps_denom;

    gpointer cef_browser;
    gpointer cef_client;
    gpointer cef_thread;

    guint8 *frame_buffer;
    gsize frame_size;
    GMutex frame_mutex;
    GCond frame_cond;
    gboolean frame_ready;
    gboolean running;
    gboolean page_loaded;

    guint64 frame_count;
};

struct _GstCefSrcClass {
    GstBinClass parent_class;
};

GType gst_cef_src_get_type(void);

G_END_DECLS

#endif
