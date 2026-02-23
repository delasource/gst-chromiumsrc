#include "gstchromiumsrc.h"
#include "cef_render_handler.h"

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>

GST_DEBUG_CATEGORY_STATIC(chromium_src_debug);
#define GST_CAT_DEFAULT chromium_src_debug

enum {
    PROP_0,
    PROP_URL,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_FRAMERATE
};

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(
        "video/x-raw, "
        "format = (string) BGRA, "
        "width = (int) [ 1, MAX ], "
        "height = (int) [ 1, MAX ], "
        "framerate = (fraction) [ 0/1, MAX ]"
    )
);

#define gst_chromium_src_parent_class parent_class
G_DEFINE_TYPE(GstChromiumSrc, gst_chromium_src, GST_TYPE_BIN);

static void gst_chromium_src_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_chromium_src_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void gst_chromium_src_finalize(GObject *object);

static GstStateChangeReturn gst_chromium_src_change_state(
    GstElement *element, GstStateChange transition);

static void gst_chromium_src_need_data(GstAppSrc *appsrc, guint length,
    gpointer user_data);
static void gst_chromium_src_enough_data(GstAppSrc *appsrc, gpointer user_data);

static void gst_chromium_src_class_init(GstChromiumSrcClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass *gstelement_class = GST_ELEMENT_CLASS(klass);

    GST_DEBUG_CATEGORY_INIT(chromium_src_debug, "chromiumsrc", 0, "Chromium Source");

    gobject_class->set_property = gst_chromium_src_set_property;
    gobject_class->get_property = gst_chromium_src_get_property;
    gobject_class->finalize = gst_chromium_src_finalize;

    g_object_class_install_property(gobject_class, PROP_URL,
        g_param_spec_string("url", "URL",
            "URL to render in the browser",
            "https://pingup.de/w/png-test.html",
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(gobject_class, PROP_WIDTH,
        g_param_spec_int("width", "Width",
            "Video width in pixels",
            1, G_MAXINT, 1920,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(gobject_class, PROP_HEIGHT,
        g_param_spec_int("height", "Height",
            "Video height in pixels",
            1, G_MAXINT, 1080,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(gobject_class, PROP_FRAMERATE,
        g_param_spec_string("framerate", "Framerate",
            "Output framerate (e.g., 1/1 for 1 fps)",
            "1/1",
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    gst_element_class_set_metadata(gstelement_class,
        "Chromium Source",
        "Source/Video",
        "Offscreen CEF/Chromium browser as video source",
        "opencode");

    gst_element_class_add_static_pad_template(gstelement_class, &src_template);

    gstelement_class->change_state = gst_chromium_src_change_state;
}

static void gst_chromium_src_init(GstChromiumSrc *src) {
    src->url = g_strdup("https://pingup.de/w/png-test.html");
    src->width = 1920;
    src->height = 1080;
    src->fps_num = 1;
    src->fps_denom = 1;

    src->appsrc = GST_APP_SRC(gst_element_factory_make("appsrc", "internal_appsrc"));
    if (!src->appsrc) {
        GST_ERROR_OBJECT(src, "Failed to create internal appsrc");
        return;
    }

    gst_bin_add(GST_BIN(src), GST_ELEMENT(src->appsrc));

    src->ghostpad = gst_ghost_pad_new("src",
        gst_element_get_static_pad(GST_ELEMENT(src->appsrc), "src"));
    gst_element_add_pad(GST_ELEMENT(src), src->ghostpad);

    g_object_set(src->appsrc, "stream-type", GST_APP_STREAM_TYPE_STREAM, NULL);
    g_object_set(src->appsrc, "format", GST_FORMAT_TIME, NULL);
    g_object_set(src->appsrc, "is-live", TRUE, NULL);
    g_object_set(src->appsrc, "emit-signals", TRUE, NULL);
    g_object_set(src->appsrc, "max-bytes", (guint64)(1920 * 1080 * 4 * 3), NULL);

    g_signal_connect(src->appsrc, "need-data",
        G_CALLBACK(gst_chromium_src_need_data), src);
    g_signal_connect(src->appsrc, "enough-data",
        G_CALLBACK(gst_chromium_src_enough_data), src);

    src->frame_buffer = NULL;
    src->frame_size = 0;
    src->frame_ready = FALSE;
    src->running = FALSE;
    src->frame_count = 0;

    g_mutex_init(&src->frame_mutex);
    g_cond_init(&src->frame_cond);

    src->cef_browser = NULL;
    src->cef_client = NULL;
    src->cef_thread = NULL;
}

static void gst_chromium_src_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(object);

    switch (prop_id) {
        case PROP_URL:
            g_free(src->url);
            src->url = g_value_dup_string(value);
            break;
        case PROP_WIDTH:
            src->width = g_value_get_int(value);
            break;
        case PROP_HEIGHT:
            src->height = g_value_get_int(value);
            break;
        case PROP_FRAMERATE: {
            const gchar *fps_str = g_value_get_string(value);
            if (fps_str) {
                if (sscanf(fps_str, "%d/%d", &src->fps_num, &src->fps_denom) != 2) {
                    src->fps_num = atoi(fps_str);
                    src->fps_denom = 1;
                }
            }
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gst_chromium_src_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(object);

    switch (prop_id) {
        case PROP_URL:
            g_value_set_string(value, src->url);
            break;
        case PROP_WIDTH:
            g_value_set_int(value, src->width);
            break;
        case PROP_HEIGHT:
            g_value_set_int(value, src->height);
            break;
        case PROP_FRAMERATE: {
            gchar *fps_str = g_strdup_printf("%d/%d", src->fps_num, src->fps_denom);
            g_value_take_string(value, fps_str);
            break;
        }
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gst_chromium_src_finalize(GObject *object) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(object);

    g_free(src->url);
    g_free(src->frame_buffer);
    g_mutex_clear(&src->frame_mutex);
    g_cond_clear(&src->frame_cond);

    G_OBJECT_CLASS(parent_class)->finalize(object);
}

static void gst_chromium_src_need_data(GstAppSrc *appsrc, guint length,
    gpointer user_data) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(user_data);
    GstBuffer *buffer;
    GstMapInfo map;
    GstFlowReturn ret;
    GstClockTime duration, timestamp;

    GST_DEBUG_OBJECT(src, "need-data: length=%u", length);

    if (!src->running) {
        return;
    }

    g_mutex_lock(&src->frame_mutex);

    while (!src->frame_ready && src->running) {
        gint64 end_time = g_get_monotonic_time() + G_TIME_SPAN_SECOND;
        if (!g_cond_wait_until(&src->frame_cond, &src->frame_mutex, end_time)) {
            GST_WARNING_OBJECT(src, "Timeout waiting for frame");
            g_mutex_unlock(&src->frame_mutex);
            return;
        }
    }

    if (!src->running || !src->frame_buffer) {
        g_mutex_unlock(&src->frame_mutex);
        return;
    }

    buffer = gst_buffer_new_and_alloc(src->frame_size);
    if (!buffer) {
        GST_ERROR_OBJECT(src, "Failed to allocate buffer");
        g_mutex_unlock(&src->frame_mutex);
        return;
    }

    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, src->frame_buffer, src->frame_size);
    gst_buffer_unmap(buffer, &map);

    src->frame_ready = FALSE;
    g_mutex_unlock(&src->frame_mutex);

    duration = gst_util_uint64_scale(GST_SECOND, src->fps_denom, src->fps_num);
    timestamp = src->frame_count * duration;

    GST_BUFFER_PTS(buffer) = timestamp;
    GST_BUFFER_DTS(buffer) = timestamp;
    GST_BUFFER_DURATION(buffer) = duration;

    src->frame_count++;

    GST_DEBUG_OBJECT(src, "Pushing buffer: ts=%" GST_TIME_FORMAT " dur=%" GST_TIME_FORMAT,
        GST_TIME_ARGS(timestamp), GST_TIME_ARGS(duration));

    ret = gst_app_src_push_buffer(src->appsrc, buffer);
    if (ret != GST_FLOW_OK) {
        GST_WARNING_OBJECT(src, "push-buffer returned %d", ret);
    }
}

static void gst_chromium_src_enough_data(GstAppSrc *appsrc, gpointer user_data) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(user_data);
    GST_DEBUG_OBJECT(src, "enough-data");
}

static gboolean gst_chromium_src_start(GstChromiumSrc *src) {
    GstCaps *caps;

    GST_INFO_OBJECT(src, "Starting Chromium source: %s", src->url);

    if (!src->url) {
        GST_ELEMENT_ERROR(src, RESOURCE, SETTINGS,
            ("No URL specified"), (NULL));
        return FALSE;
    }

    src->frame_size = src->width * src->height * 4;
    src->frame_buffer = (guint8 *)g_malloc(src->frame_size);
    if (!src->frame_buffer) {
        GST_ELEMENT_ERROR(src, RESOURCE, NO_SPACE_LEFT,
            ("Failed to allocate frame buffer"), (NULL));
        return FALSE;
    }

    caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "BGRA",
        "width", G_TYPE_INT, src->width,
        "height", G_TYPE_INT, src->height,
        "framerate", GST_TYPE_FRACTION, src->fps_num, src->fps_denom,
        NULL);

    GST_INFO_OBJECT(src, "Setting caps: %" GST_PTR_FORMAT, caps);
    gst_app_src_set_caps(src->appsrc, caps);
    gst_caps_unref(caps);

    src->running = TRUE;
    src->frame_count = 0;

    if (!cef_browser_start(src, src->url, src->width, src->height)) {
        GST_ELEMENT_ERROR(src, RESOURCE, FAILED,
            ("Failed to start CEF browser"), (NULL));
        src->running = FALSE;
        g_free(src->frame_buffer);
        src->frame_buffer = NULL;
        return FALSE;
    }

    GST_INFO_OBJECT(src, "Chromium source started successfully");
    return TRUE;
}

static gboolean gst_chromium_src_stop(GstChromiumSrc *src) {
    GST_INFO_OBJECT(src, "Stopping Chromium source");

    g_mutex_lock(&src->frame_mutex);
    src->running = FALSE;
    g_cond_signal(&src->frame_cond);
    g_mutex_unlock(&src->frame_mutex);

    cef_browser_stop(src);

    g_free(src->frame_buffer);
    src->frame_buffer = NULL;
    src->frame_size = 0;

    if (src->appsrc) {
        gst_app_src_end_of_stream(src->appsrc);
    }

    GST_INFO_OBJECT(src, "Chromium source stopped");
    return TRUE;
}

static GstStateChangeReturn gst_chromium_src_change_state(
    GstElement *element, GstStateChange transition) {
    GstChromiumSrc *src = GST_CHROMIUM_SRC(element);
    GstStateChangeReturn ret;

    switch (transition) {
        case GST_STATE_CHANGE_NULL_TO_READY:
            break;
        case GST_STATE_CHANGE_READY_TO_PAUSED:
            if (!gst_chromium_src_start(src)) {
                return GST_STATE_CHANGE_FAILURE;
            }
            break;
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            break;
        default:
            break;
    }

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);

    switch (transition) {
        case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            break;
        case GST_STATE_CHANGE_PAUSED_TO_READY:
            gst_chromium_src_stop(src);
            break;
        case GST_STATE_CHANGE_READY_TO_NULL:
            break;
        default:
            break;
    }

    return ret;
}

static gboolean plugin_init(GstPlugin *plugin) {
    return gst_element_register(plugin, "chromiumsrc", GST_RANK_NONE,
        GST_TYPE_CHROMIUM_SRC);
}

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    chromiumsrc,
    "CEF/Chromium browser as video source",
    plugin_init,
    "1.0.0",
    "LGPL",
    "chromiumsrc",
    "https://github.com/opencode/chromiumsrc"
)
