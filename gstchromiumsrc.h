#ifndef __GST_CHROMIUM_SRC_H__
#define __GST_CHROMIUM_SRC_H__

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

G_BEGIN_DECLS

#define GST_TYPE_CHROMIUM_SRC (gst_chromium_src_get_type())
#define GST_CHROMIUM_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_CHROMIUM_SRC, GstChromiumSrc))
#define GST_CHROMIUM_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_CHROMIUM_SRC, GstChromiumSrcClass))
#define GST_IS_CHROMIUM_SRC(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_CHROMIUM_SRC))
#define GST_IS_CHROMIUM_SRC_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_CHROMIUM_SRC))

typedef struct _GstChromiumSrc GstChromiumSrc;
typedef struct _GstChromiumSrcClass GstChromiumSrcClass;

struct _GstChromiumSrc {
    GstBin    parent;
    GstAppSrc *appsrc;
    GstPad    *ghostpad;

    gchar *url;
    gint  width;
    gint  height;
    gint  fps_num;
    gint  gpu_device;

    gpointer cef_browser;
    gpointer cef_client;
    GThread  *cef_thread;

    guint8   *frame_buffer;
    gsize    frame_size;
    GMutex   frame_mutex;
    GCond    frame_cond;
    gboolean frame_ready;
    gboolean running;
    gboolean page_loaded;
    gboolean gpu_enabled;

    guint64 frame_count;
};

struct _GstChromiumSrcClass {
    GstBinClass parent_class;
};

GType gst_chromium_src_get_type(void);

G_END_DECLS

#endif
