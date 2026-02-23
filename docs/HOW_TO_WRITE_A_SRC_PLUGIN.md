# Writing a GStreamer Source Plugin

This guide describes how to write a custom GStreamer source plugin for pushing image or video data into a pipeline. It covers the essential APIs, required callbacks, and the structure of a minimal but complete source element.

## Overview

A source plugin in GStreamer is an element that produces data for the pipeline. It has no sink pads—only source pads. For file-based or data-producing sources, you typically subclass `GstPushSrc` (which itself inherits from `GstBaseSrc` → `GstElement` → `GstObject`).

### Class Hierarchy

```
GObject
  ╰── GInitiallyUnowned
        ╰── GstObject
              ╰── GstElement
                    ╰── GstBaseSrc
                          ╰── GstPushSrc
                                ╰── YourSource
```

- **GstBaseSrc**: Base class for source elements. Handles state changes, seeking, and provides virtual methods for the core functionality.
- **GstPushSrc**: Subclass of GstBaseSrc that implements a push-based source. You implement the `create()` method to produce buffers.

## Plugin Structure

A minimal source plugin consists of:

1. **Type registration** - `G_DEFINE_TYPE()` macro
2. **Class initialization** - Set up properties, pad templates, and vfunc pointers
3. **Instance initialization** - Initialize member variables
4. **Virtual method implementations** - The core logic
5. **Plugin definition** - `GST_PLUGIN_DEFINE()` macro

## Essential Virtual Methods

### GstBaseSrc Methods

| Method | Required | Purpose |
|--------|----------|---------|
| `start()` | Recommended | Initialize resources (open file, allocate buffers) |
| `stop()` | Recommended | Clean up resources (close file, free memory) |
| `get_size()` | Optional | Return total stream size in bytes (enables seeking) |
| `is_seekable()` | Optional | Return `TRUE` if the source supports seeking |
| `do_seek()` | Optional* | Handle seek requests (required if `is_seekable` returns TRUE) |
| `query()` | Optional | Handle queries (position, duration, etc.) |
| `event()` | Optional | Handle events (flush, seek, etc.) |

### GstPushSrc Methods

| Method | Required | Purpose |
|--------|----------|---------|
| `create()` | **Yes** | Create and push the next buffer |
| `alloc()` | Optional | Pre-allocate buffer (rarely needed) |
| `fill()` | Optional | Fill an allocated buffer (alternative to create) |

## Pad Templates

Every source element must define at least one source pad template. This tells GStreamer what kind of data the element can produce.

```c
static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
    "src",                    // Pad name
    GST_PAD_SRC,              // Direction
    GST_PAD_ALWAYS,           // Availability
    GST_STATIC_CAPS_ANY       // Capabilities (or specific caps)
);
```

For image/video sources with specific formats:

```c
static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(
        "image/png; "
        "image/jpeg; "
        "video/x-raw, "
        "format = (string) { RGB, BGR, RGBA }, "
        "width = (int) [ 1, MAX ], "
        "height = (int) [ 1, MAX ], "
        "framerate = (fraction) [ 0/1, MAX ]"
    )
);
```

The template must be added in class_init:

```c
gst_element_class_add_static_pad_template(element_class, &src_template);
```

## Properties

Expose configuration through GObject properties. Common properties for sources:

```c
enum {
    PROP_0,
    PROP_LOCATION,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_FRAMERATE
};

static void gst_my_src_class_init(GstMySrcClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    
    g_object_class_install_property(gobject_class, PROP_LOCATION,
        g_param_spec_string("location", "File Location",
            "Path to the input file", NULL,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    
    g_object_class_install_property(gobject_class, PROP_WIDTH,
        g_param_spec_int("width", "Width",
            "Video width in pixels", 1, G_MAXINT, 1920,
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    
    // ... more properties
    
    gobject_class->set_property = gst_my_src_set_property;
    gobject_class->get_property = gst_my_src_get_property;
}
```

## The create() Method

This is where data is produced. The method must:

1. Check if end-of-stream has been reached
2. Create a GstBuffer with the data
3. Set buffer metadata (offset, timestamp if applicable)
4. Return the buffer via the output parameter

```c
static GstFlowReturn gst_my_src_create(GstPushSrc *pushsrc, GstBuffer **buffer) {
    GstMySrc *src = GST_MY_SRC(pushsrc);
    GstBuffer *buf;
    gsize to_read;
    
    // Check for EOS
    if (src->offset >= src->size) {
        return GST_FLOW_EOS;
    }
    
    // Calculate how much to read
    to_read = MIN(GST_BASE_SRC(pushsrc)->blocksize, src->size - src->offset);
    
    // Create buffer wrapping existing memory (zero-copy)
    buf = gst_buffer_new();
    gst_buffer_append_memory(buf,
        gst_memory_new_wrapped(
            GST_MEMORY_FLAG_READONLY,
            src->data + src->offset,    // Pointer to data
            to_read,                     // Max size
            0,                           // Offset within memory
            to_read,                     // Size to use
            NULL,                        // User data for destroy notify
            NULL                         // Destroy notify function
        ));
    
    // Set buffer metadata
    GST_BUFFER_OFFSET(buf) = src->offset;
    GST_BUFFER_OFFSET_END(buf) = src->offset + to_read;
    
    // Advance position
    src->offset += to_read;
    
    *buffer = buf;
    return GST_FLOW_OK;
}
```

### GstFlowReturn Values

| Value | Meaning |
|-------|---------|
| `GST_FLOW_OK` | Success, buffer produced |
| `GST_FLOW_EOS` | End of stream reached |
| `GST_FLOW_ERROR` | Generic error |
| `GST_FLOW_NOT_NEGOTIATED` | Format negotiation failed |
| `GST_FLOW_FLUSHING` | Element is shutting down |
| `GST_FLOW_NOT_SUPPORTED` | Operation not supported |

## Creating Buffers

### Method 1: Wrapping Existing Memory (Zero-Copy)

Use when you already have data in memory and want to avoid copying:

```c
buf = gst_buffer_new();
gst_buffer_append_memory(buf,
    gst_memory_new_wrapped(
        GST_MEMORY_FLAG_READONLY,
        data_ptr,
        data_size,
        0,           // offset
        data_size,   // size to use
        NULL, NULL   // optional destroy notify
    ));
```

### Method 2: Allocating New Memory

Use when you need to fill a new buffer:

```c
buf = gst_buffer_new_allocate(NULL, size, NULL);
gst_buffer_fill(buf, 0, data, size);
```

### Method 3: Using GstMapInfo (Read/Write Access)

For complex buffer manipulation:

```c
GstMapInfo map;
buf = gst_buffer_new_and_alloc(size);
gst_buffer_map(buf, &map, GST_MAP_WRITE);
memcpy(map.data, source_data, size);
gst_buffer_unmap(buf, &map);
```

## Seeking Support

For seekable sources (files, not live streams):

```c
static gboolean gst_my_src_is_seekable(GstBaseSrc *basesrc) {
    return TRUE;
}

static gboolean gst_my_src_get_size(GstBaseSrc *basesrc, guint64 *size) {
    GstMySrc *src = GST_MY_SRC(basesrc);
    *size = src->size;
    return TRUE;
}

static gboolean gst_my_src_do_seek(GstBaseSrc *basesrc, GstSegment *segment) {
    GstMySrc *src = GST_MY_SRC(basesrc);
    
    // Validate seek position
    if (segment->start > src->size) {
        return FALSE;
    }
    
    // Update read position
    src->offset = segment->start;
    return TRUE;
}
```

## Timestamps

For video sources with timing information:

```c
static GstFlowReturn gst_my_src_create(GstPushSrc *pushsrc, GstBuffer **buffer) {
    GstMySrc *src = GST_MY_SRC(pushsrc);
    GstBuffer *buf;
    GstClockTime duration, timestamp;
    
    // ... create buffer ...
    
    // Calculate duration based on framerate
    duration = gst_util_uint64_scale(GST_SECOND, src->framerate_denom, src->framerate_num);
    timestamp = src->frame_count * duration;
    
    GST_BUFFER_PTS(buf) = timestamp;
    GST_BUFFER_DURATION(buf) = duration;
    
    src->frame_count++;
    
    *buffer = buf;
    return GST_FLOW_OK;
}
```

For static images or files without timing, you can omit timestamps or use `GST_CLOCK_TIME_NONE`.

## Caps Negotiation

For sources that produce format-specific data, you may need to set caps:

### In start() (Fixed Format)

```c
static gboolean gst_my_src_start(GstBaseSrc *basesrc) {
    GstMySrc *src = GST_MY_SRC(basesrc);
    GstCaps *caps;
    
    // ... load data, determine format ...
    
    caps = gst_caps_new_simple("image/png", NULL);
    gst_base_src_set_caps(basesrc, caps);
    gst_caps_unref(caps);
    
    return TRUE;
}
```

### Dynamic Caps (Variable Format)

```c
caps = gst_caps_new_simple("video/x-raw",
    "format", G_TYPE_STRING, "RGB",
    "width", G_TYPE_INT, src->width,
    "height", G_TYPE_INT, src->height,
    "framerate", GST_TYPE_FRACTION, src->fps_num, src->fps_denom,
    NULL);
gst_base_src_set_caps(basesrc, caps);
gst_caps_unref(caps);
```

## Live Sources

For live sources (cameras, network streams), mark the source as live:

```c
static gboolean gst_my_src_start(GstBaseSrc *basesrc) {
    // ... initialization ...
    
    gst_base_src_set_live(basesrc, TRUE);
    gst_base_src_set_format(basesrc, GST_FORMAT_TIME);
    
    return TRUE;
}
```

Live sources:
- Cannot be paused (transition to READY instead of PAUSED)
- Use `GST_FORMAT_TIME` for timing
- May need to handle latency with `gst_base_src_set_latency()`

## Plugin Registration

### Element Registration

```c
static gboolean plugin_init(GstPlugin *plugin) {
    return gst_element_register(plugin, "mysrc", GST_RANK_NONE, GST_TYPE_MY_SRC);
}
```

The rank determines plugin priority when autoplugging:
- `GST_RANK_NONE` (0): Never use automatically
- `GST_RANK_MARGINAL` (64): Use as fallback
- `GST_RANK_SECONDARY` (128): Prefer over marginal
- `GST_RANK_PRIMARY` (256): Best choice

### Plugin Definition

```c
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,      // GStreamer major version
    GST_VERSION_MINOR,      // GStreamer minor version
    mysrc,                  // Plugin name (identifier)
    "My custom source",     // Description
    plugin_init,            // Init function
    "1.0",                  // Version
    "LGPL",                 // License
    "myproject",            // Package name
    "https://example.com"   // Origin URL
)
```

## Building the Plugin

### Compilation Flags

```makefile
CFLAGS=$(shell pkg-config --cflags gstreamer-1.0 gstreamer-base-1.0)
PLUGIN_CFLAGS=$(CFLAGS) -DPACKAGE=\"mysrc\" -DPACKAGE_VERSION=\"1.0\"
LIBS=$(shell pkg-config --libs gstreamer-1.0 gstreamer-base-1.0)

libgstmysrc.so: mysrc.c
	$(CC) $(PLUGIN_CFLAGS) -shared -fPIC -o $@ $< $(LIBS)
```

Required flags:
- `-shared -fPIC`: Build as shared library
- `-DPACKAGE`: Required by `GST_PLUGIN_DEFINE`
- `-DPACKAGE_VERSION`: Required by `GST_PLUGIN_DEFINE`

### Installation

```bash
# User-local installation
mkdir -p ~/.local/share/gstreamer-1.0/plugins
cp libgstmysrc.so ~/.local/share/gstreamer-1.0/plugins/

# System-wide (requires root)
cp libgstmysrc.so /usr/lib/x86_64-linux-gnu/gstreamer-1.0/
gst-plugin-scanner --update-registry
```

## Testing

### Inspect Plugin

```bash
GST_PLUGIN_PATH=/path/to/plugin/dir gst-inspect-1.0 mysrc
```

### Test with fakesink

```bash
GST_PLUGIN_PATH=/path/to/plugin/dir gst-launch-1.0 \
    mysrc location=test.png ! fakesink -v
```

### Test with Full Pipeline

```bash
GST_PLUGIN_PATH=/path/to/plugin/dir gst-launch-1.0 \
    mysrc location=test.png ! decodebin ! videoconvert ! autovideosink
```

### Debug Output

```bash
GST_DEBUG=mysrc:5 gst-launch-1.0 mysrc ! fakesink
```

Debug levels: 1=error, 2=warning, 3=fixme, 4=info, 5=debug, 9=log

## Complete Example Structure

```c
#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#define GST_TYPE_MY_SRC (gst_my_src_get_type())
G_DECLARE_FINAL_TYPE(GstMySrc, gst_my_src, GST, MY_SRC, GstPushSrc)

struct _GstMySrc {
    GstPushSrc parent;
    gchar *location;
    guint8 *data;
    gsize size;
    gsize offset;
};

G_DEFINE_TYPE(GstMySrc, gst_my_src, GST_TYPE_PUSH_SRC)

static void gst_my_src_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_my_src_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);
static void gst_my_src_finalize(GObject *object);

static gboolean gst_my_src_start(GstBaseSrc *basesrc);
static gboolean gst_my_src_stop(GstBaseSrc *basesrc);
static gboolean gst_my_src_get_size(GstBaseSrc *basesrc, guint64 *size);
static gboolean gst_my_src_is_seekable(GstBaseSrc *basesrc);
static gboolean gst_my_src_do_seek(GstBaseSrc *basesrc, GstSegment *segment);
static GstFlowReturn gst_my_src_create(GstPushSrc *pushsrc, GstBuffer **buffer);

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE(
    "src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS_ANY);

static void gst_my_src_class_init(GstMySrcClass *klass) {
    GObjectClass *gobject = G_OBJECT_CLASS(klass);
    GstElementClass *element = GST_ELEMENT_CLASS(klass);
    GstBaseSrcClass *basesrc = GST_BASE_SRC_CLASS(klass);
    GstPushSrcClass *pushsrc = GST_PUSH_SRC_CLASS(klass);
    
    gobject->set_property = gst_my_src_set_property;
    gobject->get_property = gst_my_src_get_property;
    gobject->finalize = gst_my_src_finalize;
    
    g_object_class_install_property(gobject, PROP_LOCATION,
        g_param_spec_string("location", "Location",
            "File path", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    
    gst_element_class_set_metadata(element,
        "My Source", "Source/File", "Description", "Author");
    gst_element_class_add_static_pad_template(element, &src_template);
    
    basesrc->start = GST_DEBUG_FUNCPTR(gst_my_src_start);
    basesrc->stop = GST_DEBUG_FUNCPTR(gst_my_src_stop);
    basesrc->get_size = GST_DEBUG_FUNCPTR(gst_my_src_get_size);
    basesrc->is_seekable = GST_DEBUG_FUNCPTR(gst_my_src_is_seekable);
    basesrc->do_seek = GST_DEBUG_FUNCPTR(gst_my_src_do_seek);
    
    pushsrc->create = GST_DEBUG_FUNCPTR(gst_my_src_create);
}

static void gst_my_src_init(GstMySrc *src) {
    gst_base_src_set_format(GST_BASE_SRC(src), GST_FORMAT_BYTES);
}

// ... implement all methods ...

static gboolean plugin_init(GstPlugin *plugin) {
    return gst_element_register(plugin, "mysrc", GST_RANK_NONE, GST_TYPE_MY_SRC);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, mysrc,
    "My Source Plugin", plugin_init, "1.0", "LGPL", "myproject", "https://example.com")
```

## Common Patterns

### Producing Raw Video Frames

```c
// In start(), set caps for raw video
GstCaps *caps = gst_caps_new_simple("video/x-raw",
    "format", G_TYPE_STRING, "RGB",
    "width", G_TYPE_INT, 1920,
    "height", G_TYPE_INT, 1080,
    "framerate", GST_TYPE_FRACTION, 30, 1,
    NULL);
gst_base_src_set_caps(basesrc, caps);

// In create(), produce frames with timing
GST_BUFFER_PTS(buf) = src->frame_num * GST_SECOND / 30;
GST_BUFFER_DURATION(buf) = GST_SECOND / 30;
```

### Reading from a File Descriptor or Socket

```c
static GstFlowReturn gst_my_src_create(GstPushSrc *pushsrc, GstBuffer **buffer) {
    GstMySrc *src = GST_MY_SRC(pushsrc);
    GstBuffer *buf;
    guint8 data[MAX_CHUNK];
    ssize_t bytes_read;
    
    bytes_read = read(src->fd, data, sizeof(data));
    
    if (bytes_read == 0) {
        return GST_FLOW_EOS;
    }
    if (bytes_read < 0) {
        GST_ELEMENT_ERROR(src, RESOURCE, READ, ("Read error"), (NULL));
        return GST_FLOW_ERROR;
    }
    
    buf = gst_buffer_new_wrapped_full(GST_MEMORY_FLAG_READONLY,
        data, bytes_read, 0, bytes_read, NULL, NULL);
    *buffer = buf;
    return GST_FLOW_OK;
}
```

### Handling Format Changes

```c
static GstFlowReturn gst_my_src_create(GstPushSrc *pushsrc, GstBuffer **buffer) {
    GstMySrc *src = GST_MY_SRC(pushsrc);
    
    // Detect format change
    if (src->format_changed) {
        GstCaps *new_caps = gst_caps_new_simple(...);
        gst_base_src_set_caps(GST_BASE_SRC(pushsrc), new_caps);
        gst_caps_unref(new_caps);
        src->format_changed = FALSE;
    }
    
    // ... create buffer ...
}
```

## Debugging Macros

Use GStreamer's debug system for consistent logging:

```c
GST_DEBUG_OBJECT(src, "Processing frame %d", frame_num);
GST_INFO_OBJECT(src, "Loaded file: %s (%zu bytes)", filename, size);
GST_WARNING_OBJECT(src, "Unexpected format: %s", format);
GST_ERROR_OBJECT(src, "Failed to read: %s", error);
```

Enable in code:
```c
GST_DEBUG_CATEGORY_STATIC(my_src_debug);
#define GST_CAT_DEFAULT my_src_debug

static void gst_my_src_class_init(GstMySrcClass *klass) {
    GST_DEBUG_CATEGORY_INIT(my_src_debug, "mysrc", 0, "My Source");
    // ...
}
```

## References

- Find documentation in this directory (.md files)

