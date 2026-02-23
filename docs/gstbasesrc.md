Title: GstBaseSrc

URL Source: https://gstreamer.freedesktop.org/documentation/base/gstbasesrc.html

Markdown Content:
# GstBaseSrc

This is a generic base class for source elements. The following types of sources are supported:

* random access sources like files
* seekable sources
* live sources

The source can be configured to operate in any [GstFormat](gstreamer/gstformat.html#GstFormat) with the[gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) method. The currently set format determines the format of the internal [GstSegment](gstreamer/gstsegment.html#GstSegment) and any [GST\_EVENT\_SEGMENT](gstreamer/gstevent.html#GST%5FEVENT%5FSEGMENT)events. The default format for [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) is [GST\_FORMAT\_BYTES](gstreamer/gstformat.html#GST%5FFORMAT%5FBYTES).

[GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) always supports push mode scheduling. If the following conditions are met, it also supports pull mode scheduling:

* The format is set to [GST\_FORMAT\_BYTES](gstreamer/gstformat.html#GST%5FFORMAT%5FBYTES) (default).
* [is\_seekable](base/gstbasesrc.html#GstBaseSrcClass::is%5Fseekable) returns [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS).

If all the conditions are met for operating in pull mode, [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) is automatically seekable in push mode as well. The following conditions must be met to make the element seekable in push mode when the format is not[GST\_FORMAT\_BYTES](gstreamer/gstformat.html#GST%5FFORMAT%5FBYTES):

* [is\_seekable](base/gstbasesrc.html#GstBaseSrcClass::is%5Fseekable) returns [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS).
* [query](base/gstbasesrc.html#GstBaseSrcClass::query) can convert all supported seek formats to the internal format as set with [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).
* [do\_seek](base/gstbasesrc.html#GstBaseSrcClass::do%5Fseek) is implemented, performs the seek and returns[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS).

When the element does not meet the requirements to operate in pull mode, the offset and length in the [create](base/gstbasesrc.html#GstBaseSrcClass::create) method should be ignored. It is recommended to subclass [GstPushSrc](base/gstpushsrc.html#GstPushSrc) instead, in this situation. If the element can operate in pull mode but only with specific offsets and lengths, it is allowed to generate an error when the wrong values are passed to the [create](base/gstbasesrc.html#GstBaseSrcClass::create) function.

[GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) has support for live sources. Live sources are sources that when paused discard data, such as audio or video capture devices. A typical live source also produces data at a fixed rate and thus provides a clock to publish this rate. Use [gst\_base\_src\_set\_live](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Flive) to activate the live source mode.

A live source does not produce data in the PAUSED state. This means that the[create](base/gstbasesrc.html#GstBaseSrcClass::create) method will not be called in PAUSED but only in PLAYING. To signal the pipeline that the element will not produce data, the return value from the READY to PAUSED state will be[GST\_STATE\_CHANGE\_NO\_PREROLL](gstreamer/gstelement.html#GST%5FSTATE%5FCHANGE%5FNO%5FPREROLL).

A typical live source will timestamp the buffers it creates with the current running time of the pipeline. This is one reason why a live source can only produce data in the PLAYING state, when the clock is actually distributed and running.

Live sources that synchronize and block on the clock (an audio source, for example) can use [gst\_base\_src\_wait\_playing](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fwait%5Fplaying) when the[create](base/gstbasesrc.html#GstBaseSrcClass::create) function was interrupted by a state change to PAUSED.

The [get\_times](base/gstbasesrc.html#GstBaseSrcClass::get%5Ftimes) method can be used to implement pseudo-live sources. It only makes sense to implement the [get\_times](base/gstbasesrc.html#GstBaseSrcClass::get%5Ftimes)function if the source is a live source. The [get\_times](base/gstbasesrc.html#GstBaseSrcClass::get%5Ftimes)function should return timestamps starting from 0, as if it were a non-live source. The base class will make sure that the timestamps are transformed into the current running\_time. The base source will then wait for the calculated running\_time before pushing out the buffer.

For live sources, the base class will by default report a latency of 0\. For pseudo live sources, the base class will by default measure the difference between the first buffer timestamp and the start time of get\_times and will report this value as the latency. Subclasses should override the query function when this behaviour is not acceptable.

There is only support in [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) for exactly one source pad, which should be named "src". A source implementation (subclass of [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)) should install a pad template in its class\_init function, like so:

```[<!--
 static void
 my_element_class_init (GstMyElementClass *klass)
 {
   GstElementClass *gstelement_class = GST_ELEMENT_CLASS (klass);
   // srctemplate should be a #GstStaticPadTemplate with direction
   // %GST_PAD_SRC and name "src"
   gst_element_class_add_static_pad_template (gstelement_class, &srctemplate);

   gst_element_class_set_static_metadata (gstelement_class,
      "Source name",
      "Source",
      "My Source element",
      "The author <my.sink@my.email>");
 }

```

## Controlled shutdown of live sources in applications

Applications that record from a live source may want to stop recording in a controlled way, so that the recording is stopped, but the data already in the pipeline is processed to the end (remember that many live sources would go on recording forever otherwise). For that to happen the application needs to make the source stop recording and send an EOS event down the pipeline. The application would then wait for an EOS message posted on the pipeline's bus to know when all data has been processed and the pipeline can safely be stopped.

An application may send an EOS event to a source element to make it perform the EOS logic (send EOS event downstream or post a[GST\_MESSAGE\_SEGMENT\_DONE](gstreamer/gstmessage.html#GST%5FMESSAGE%5FSEGMENT%5FDONE) on the bus). This can typically be done with the [gst\_element\_send\_event](gstreamer/gstelement.html#gst%5Felement%5Fsend%5Fevent) function on the element or its parent bin.

After the EOS has been sent to the element, the application should wait for an EOS message to be posted on the pipeline's bus. Once this EOS message is received, it may safely shut down the entire pipeline.

## _GstBaseSrc_ 


[GObject](https://docs.gtk.org/gobject/class.Object.html "GObject")
    ╰──[GInitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GInitiallyUnowned")
        ╰──[GstObject](gstreamer/gstobject.html#GstObject "GstObject")
            ╰──[GstElement](gstreamer/gstelement.html#GstElement "GstElement")
                ╰──GstBaseSrc
                    ╰──[GstPushSrc](base/gstpushsrc.html#GstPushSrc "GstPushSrc")

The opaque [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) data structure.

### Members

__`element`_ ([GstElement](gstreamer/gstelement.html#GstElement "GstElement")) –

_No description available_ 

__`srcpad`_ ([GstPad](gstreamer/gstpad.html#GstPad "GstPad") \*) –

_No description available_ 

__`livelock`_ ([GMutex](https://docs.gtk.org/glib/union.Mutex.html "GMutex")) –

_No description available_ 

__`livecond`_ ([GCond](https://docs.gtk.org/glib/struct.Cond.html "GCond")) –

_No description available_ 

__`islive`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`liverunning`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`blocksize`_ ([guint](https://docs.gtk.org/glib/types.html#guint "guint")) –

_No description available_ 

__`canactivatepush`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`randomaccess`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`clockid`_ ([GstClockID](gstreamer/gstclock.html#GstClockID "GstClockID")) –

_No description available_ 

__`segment`_ ([GstSegment](gstreamer/gstsegment.html#GstSegment "GstSegment")) –

_No description available_ 

__`neednewsegment`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`numbuffers`_ ([gint](https://docs.gtk.org/glib/types.html#gint "gint")) –

_No description available_ 

__`numbuffersleft`_ ([gint](https://docs.gtk.org/glib/types.html#gint "gint")) –

_No description available_ 

__`typefind`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`running`_ ([gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")) –

_No description available_ 

__`pendingseek`_ ([GstEvent](gstreamer/gstevent.html#GstEvent "GstEvent") \*) –

_No description available_ 

__`priv`_ (GstBaseSrcPrivate\*) –

_No description available_ 

---

### Class structure

#### _GstBaseSrcClass_ 

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _create_ method should be overridden to produce buffers.

##### Fields

__`parentclass`_ (<GstElementClass>) –

Element parent class

---

###  GstBase.BaseSrcClass

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _create_ method should be overridden to produce buffers.

##### Attributes

__`parentclass`_ ([Gst.ElementClass](GstElementClass "Gst.ElementClass")) –

Element parent class

---

###  GstBase.BaseSrcClass

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _create_ method should be overridden to produce buffers.

##### Attributes

__`parentclass`_ ([Gst.ElementClass](GstElementClass "Gst.ElementClass")) –

Element parent class

---

## _GstBase.BaseSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──GstBase.BaseSrc
                    ╰──[GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc")

The opaque [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) data structure.

### Members

__`element`_ ([Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")) –

_No description available_ 

__`srcpad`_ ([Gst.Pad](gstreamer/gstpad.html#GstPad "Gst.Pad")) –

_No description available_ 

__`livelock`_ ([GLib.Mutex](https://docs.gtk.org/glib/union.Mutex.html "GLib.Mutex")) –

_No description available_ 

__`livecond`_ ([GLib.Cond](https://docs.gtk.org/glib/struct.Cond.html "GLib.Cond")) –

_No description available_ 

__`islive`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`liverunning`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`blocksize`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`canactivatepush`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`randomaccess`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`clockid`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")) –

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")) –

_No description available_ 

__`neednewsegment`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`numbuffers`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`numbuffersleft`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`typefind`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`running`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")) –

_No description available_ 

__`pendingseek`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event")) –

_No description available_ 

__`priv`_ (GstBase.BaseSrcPrivate) –

_No description available_ 

---

## _GstBase.BaseSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──GstBase.BaseSrc
                    ╰──[GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc")

The opaque [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) data structure.

### Members

__`element`_ ([Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")) –

_No description available_ 

__`srcpad`_ ([Gst.Pad](gstreamer/gstpad.html#GstPad "Gst.Pad")) –

_No description available_ 

__`livelock`_ ([GLib.Mutex](https://docs.gtk.org/glib/union.Mutex.html "GLib.Mutex")) –

_No description available_ 

__`livecond`_ ([GLib.Cond](https://docs.gtk.org/glib/struct.Cond.html "GLib.Cond")) –

_No description available_ 

__`islive`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`liverunning`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`blocksize`_ ([int](https://docs.python.org/3/library/functions.html#int "int")) –

_No description available_ 

__`canactivatepush`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`randomaccess`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`clockid`_ ([object](https://docs.python.org/3/library/functions.html#object "object")) –

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")) –

_No description available_ 

__`neednewsegment`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`numbuffers`_ ([int](https://docs.python.org/3/library/functions.html#int "int")) –

_No description available_ 

__`numbuffersleft`_ ([int](https://docs.python.org/3/library/functions.html#int "int")) –

_No description available_ 

__`typefind`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`running`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool")) –

_No description available_ 

__`pendingseek`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event")) –

_No description available_ 

__`priv`_ (GstBase.BaseSrcPrivate) –

_No description available_ 

---

### Methods

#### _gst\_base\_src\_get\_allocator_ 


gst_base_src_get_allocator ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                            [GstAllocator](gstreamer/gstallocator.html#GstAllocator "GstAllocator") ** allocator,
                            [GstAllocationParams](gstreamer/gstallocator.html#GstAllocationParams "GstAllocationParams") * params)

Lets [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) sub-classes to know the memory _allocator_used by the base class and its _params_.

Unref the _allocator_ after usage.

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`allocator`_ ( \[out\]\[optional\]\[nullable\]\[transfer: full\])–

the [GstAllocator](gstreamer/gstallocator.html#GstAllocator)used

__`params`_ ( \[out\]\[optional\])–

the [GstAllocationParams](gstreamer/gstallocator.html#GstAllocationParams) of _allocator_

---

#### _GstBase.BaseSrc.prototype.get\_allocator_ 


`function GstBase.BaseSrc.prototype.get_allocator(): {
    // javascript wrapper for 'gst_base_src_get_allocator'
}`

Lets [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) sub-classes to know the memory _allocator_used by the base class and its _params_.

Unref the _allocator_ after usage.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

---

#### _GstBase.BaseSrc.get\_allocator_ 


`def GstBase.BaseSrc.get_allocator (self):
    #python wrapper for 'gst_base_src_get_allocator'`

Lets [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) sub-classes to know the memory _allocator_used by the base class and its _params_.

Unref the _allocator_ after usage.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

---

#### _gst\_base\_src\_get\_blocksize_ 


[guint](https://docs.gtk.org/glib/types.html#guint "guint")
gst_base_src_get_blocksize ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Get the number of bytes that _src_ will push out with each buffer.

**Parameters:**

__`src`_–

the source

**Returns**–

the number of bytes pushed with each buffer.

---

#### _GstBase.BaseSrc.prototype.get\_blocksize_ 


`function GstBase.BaseSrc.prototype.get_blocksize(): {
    // javascript wrapper for 'gst_base_src_get_blocksize'
}`

Get the number of bytes that _src_ will push out with each buffer.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the number of bytes pushed with each buffer.

---

#### _GstBase.BaseSrc.get\_blocksize_ 


`def GstBase.BaseSrc.get_blocksize (self):
    #python wrapper for 'gst_base_src_get_blocksize'`

Get the number of bytes that _src_ will push out with each buffer.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the number of bytes pushed with each buffer.

---

#### _gst\_base\_src\_get\_buffer\_pool_ 


[GstBufferPool](gstreamer/gstbufferpool.html#GstBufferPool "GstBufferPool") *
gst_base_src_get_buffer_pool ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

**Returns** ( \[nullable\]\[transfer: full\])–

the instance of the [GstBufferPool](gstreamer/gstbufferpool.html#GstBufferPool) used by the src; unref it after usage.

---

#### _GstBase.BaseSrc.prototype.get\_buffer\_pool_ 


`function GstBase.BaseSrc.prototype.get_buffer_pool(): {
    // javascript wrapper for 'gst_base_src_get_buffer_pool'
}`

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

**Returns** ([Gst.BufferPool](gstreamer/gstbufferpool.html#GstBufferPool "Gst.BufferPool"))–

the instance of the [Gst.BufferPool](gstreamer/gstbufferpool.html#GstBufferPool) used by the src; unref it after usage.

---

#### _GstBase.BaseSrc.get\_buffer\_pool_ 


`def GstBase.BaseSrc.get_buffer_pool (self):
    #python wrapper for 'gst_base_src_get_buffer_pool'`

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

**Returns** ([Gst.BufferPool](gstreamer/gstbufferpool.html#GstBufferPool "Gst.BufferPool"))–

the instance of the [Gst.BufferPool](gstreamer/gstbufferpool.html#GstBufferPool) used by the src; unref it after usage.

---

#### _gst\_base\_src\_get\_do\_timestamp_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_get_do_timestamp ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Query if _src_ timestamps outgoing buffers based on the current running\_time.

**Parameters:**

__`src`_–

the source

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the base class will automatically timestamp outgoing buffers.

---

#### _GstBase.BaseSrc.prototype.get\_do\_timestamp_ 


`function GstBase.BaseSrc.prototype.get_do_timestamp(): {
    // javascript wrapper for 'gst_base_src_get_do_timestamp'
}`

Query if _src_ timestamps outgoing buffers based on the current running\_time.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the base class will automatically timestamp outgoing buffers.

---

#### _GstBase.BaseSrc.get\_do\_timestamp_ 


`def GstBase.BaseSrc.get_do_timestamp (self):
    #python wrapper for 'gst_base_src_get_do_timestamp'`

Query if _src_ timestamps outgoing buffers based on the current running\_time.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if the base class will automatically timestamp outgoing buffers.

---

#### _gst\_base\_src\_is\_async_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_is_async ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Get the current async behaviour of _src_. See also [gst\_base\_src\_set\_async](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fasync).

**Parameters:**

__`src`_–

base source instance

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if _src_ is operating in async mode.

---

#### _GstBase.BaseSrc.prototype.is\_async_ 


`function GstBase.BaseSrc.prototype.is_async(): {
    // javascript wrapper for 'gst_base_src_is_async'
}`

Get the current async behaviour of _src_. See also [GstBase.BaseSrc.prototype.set\_async](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fasync).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if _src_ is operating in async mode.

---

#### _GstBase.BaseSrc.is\_async_ 


`def GstBase.BaseSrc.is_async (self):
    #python wrapper for 'gst_base_src_is_async'`

Get the current async behaviour of _src_. See also [GstBase.BaseSrc.set\_async](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fasync).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if _src_ is operating in async mode.

---

#### _gst\_base\_src\_is\_live_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_is_live ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Check if an element is in live mode.

**Parameters:**

__`src`_–

base source instance

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if element is in live mode.

---

#### _GstBase.BaseSrc.prototype.is\_live_ 


`function GstBase.BaseSrc.prototype.is_live(): {
    // javascript wrapper for 'gst_base_src_is_live'
}`

Check if an element is in live mode.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if element is in live mode.

---

#### _GstBase.BaseSrc.is\_live_ 


`def GstBase.BaseSrc.is_live (self):
    #python wrapper for 'gst_base_src_is_live'`

Check if an element is in live mode.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if element is in live mode.

---

#### _gst\_base\_src\_negotiate_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_negotiate ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Negotiates src pad caps with downstream elements. Unmarks GST\_PAD\_FLAG\_NEED\_RECONFIGURE in any case. But marks it again if [negotiate](base/gstbasesrc.html#GstBaseSrcClass::negotiate) fails.

Do not call this in the [fill](base/gstbasesrc.html#GstBaseSrcClass::fill) vmethod. Call this in[create](base/gstbasesrc.html#GstBaseSrcClass::create) or in [alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc), _before_ any buffer is allocated.

**Parameters:**

__`src`_–

base source instance

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the negotiation succeeded, else [FALSE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#FALSE:CAPS).

**Since** : 1.18

---

#### _GstBase.BaseSrc.prototype.negotiate_ 


`function GstBase.BaseSrc.prototype.negotiate(): {
    // javascript wrapper for 'gst_base_src_negotiate'
}`

Negotiates src pad caps with downstream elements. Unmarks GST\_PAD\_FLAG\_NEED\_RECONFIGURE in any case. But marks it again if [vfunc\_negotiate](base/gstbasesrc.html#GstBaseSrcClass::negotiate) fails.

Do not call this in the [vfunc\_fill](base/gstbasesrc.html#GstBaseSrcClass::fill) vmethod. Call this in[vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) or in [vfunc\_alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc), _before_ any buffer is allocated.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the negotiation succeeded, else [false](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean).

**Since** : 1.18

---

#### _GstBase.BaseSrc.negotiate_ 


`def GstBase.BaseSrc.negotiate (self):
    #python wrapper for 'gst_base_src_negotiate'`

Negotiates src pad caps with downstream elements. Unmarks GST\_PAD\_FLAG\_NEED\_RECONFIGURE in any case. But marks it again if [do\_negotiate](base/gstbasesrc.html#GstBaseSrcClass::negotiate) fails.

Do not call this in the [do\_fill](base/gstbasesrc.html#GstBaseSrcClass::fill) vmethod. Call this in[do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) or in [do\_alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc), _before_ any buffer is allocated.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if the negotiation succeeded, else [False](https://docs.python.org/3/library/constants.html#False).

**Since** : 1.18

---

#### _gst\_base\_src\_new\_seamless\_segment_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_new_seamless_segment ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                                   [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64") start,
                                   [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64") stop,
                                   [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64") time)

Prepare a new seamless segment for emission downstream. This function must only be called by derived sub-classes, and only from the [create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the new segment will be the current format of the source, as configured with [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat)

**Parameters:**

__`src`_–

The source

__`start`_–

The new start value for the segment

__`stop`_–

Stop value for the new segment

__`time`_–

The new time value for the start of the new segment

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if preparation of the seamless segment succeeded.

**deprecated** : 1.18: Use gst\_base\_src\_new\_segment()

---

#### _GstBase.BaseSrc.prototype.new\_seamless\_segment_ 


`function GstBase.BaseSrc.prototype.new_seamless_segment(start: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), stop: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), time: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_new_seamless_segment'
}`

Prepare a new seamless segment for emission downstream. This function must only be called by derived sub-classes, and only from the [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the new segment will be the current format of the source, as configured with [GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat)

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

The source

__`start`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The new start value for the segment

__`stop`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

Stop value for the new segment

__`time`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The new time value for the start of the new segment

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if preparation of the seamless segment succeeded.

**deprecated** : 1.18: Use gst\_base\_src\_new\_segment()

---

#### _GstBase.BaseSrc.new\_seamless\_segment_ 


`def GstBase.BaseSrc.new_seamless_segment (self, start, stop, time):
    #python wrapper for 'gst_base_src_new_seamless_segment'`

Prepare a new seamless segment for emission downstream. This function must only be called by derived sub-classes, and only from the [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the new segment will be the current format of the source, as configured with [GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat)

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

The source

__`start`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The new start value for the segment

__`stop`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

Stop value for the new segment

__`time`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The new time value for the start of the new segment

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if preparation of the seamless segment succeeded.

**deprecated** : 1.18: Use gst\_base\_src\_new\_segment()

---

#### _gst\_base\_src\_new\_segment_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_new_segment ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                          const [GstSegment](gstreamer/gstsegment.html#GstSegment "GstSegment") * segment)

Prepare a new segment for emission downstream. This function must only be called by derived sub-classes, and only from the [create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the _segment_ must be identical with the current format of the source, as configured with [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [GST\_FORMAT\_UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_–

a pointer to a [GstSegment](gstreamer/gstsegment.html#GstSegment)

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if preparation of new segment succeeded.

**Since** : 1.18

---

#### _GstBase.BaseSrc.prototype.new\_segment_ 


`function GstBase.BaseSrc.prototype.new_segment(segment: [Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")): {
    // javascript wrapper for 'gst_base_src_new_segment'
}`

Prepare a new segment for emission downstream. This function must only be called by derived sub-classes, and only from the [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the _segment_ must be identical with the current format of the source, as configured with [GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [Gst.Format.UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

a pointer to a [Gst.Segment](gstreamer/gstsegment.html#GstSegment)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if preparation of new segment succeeded.

**Since** : 1.18

---

#### _GstBase.BaseSrc.new\_segment_ 


`def GstBase.BaseSrc.new_segment (self, segment):
    #python wrapper for 'gst_base_src_new_segment'`

Prepare a new segment for emission downstream. This function must only be called by derived sub-classes, and only from the [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held.

The format for the _segment_ must be identical with the current format of the source, as configured with [GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [Gst.Format.UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

a pointer to a [Gst.Segment](gstreamer/gstsegment.html#GstSegment)

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if preparation of new segment succeeded.

**Since** : 1.18

---

#### _gst\_base\_src\_push\_segment_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_push_segment ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                           const [GstSegment](gstreamer/gstsegment.html#GstSegment "GstSegment") * segment)

Send a new segment downstream. This function must only be called by derived sub-classes, and only from the [create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held. This method also requires that an out caps has been configured, so[gst\_base\_src\_set\_caps](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fcaps) needs to have been called before.

The format for the _segment_ must be identical with the current format of the source, as configured with [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [GST\_FORMAT\_UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

This is a variant of [gst\_base\_src\_new\_segment](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fnew%5Fsegment) sending the segment right away, which can be useful to ensure events ordering.

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_–

a pointer to a [GstSegment](gstreamer/gstsegment.html#GstSegment)

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if sending of new segment succeeded.

**Since** : 1.24

---

#### _GstBase.BaseSrc.prototype.push\_segment_ 


`function GstBase.BaseSrc.prototype.push_segment(segment: [Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")): {
    // javascript wrapper for 'gst_base_src_push_segment'
}`

Send a new segment downstream. This function must only be called by derived sub-classes, and only from the [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held. This method also requires that an out caps has been configured, so[GstBase.BaseSrc.prototype.set\_caps](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fcaps) needs to have been called before.

The format for the _segment_ must be identical with the current format of the source, as configured with [GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [Gst.Format.UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

This is a variant of [GstBase.BaseSrc.prototype.new\_segment](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fnew%5Fsegment) sending the segment right away, which can be useful to ensure events ordering.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

a pointer to a [Gst.Segment](gstreamer/gstsegment.html#GstSegment)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if sending of new segment succeeded.

**Since** : 1.24

---

#### _GstBase.BaseSrc.push\_segment_ 


`def GstBase.BaseSrc.push_segment (self, segment):
    #python wrapper for 'gst_base_src_push_segment'`

Send a new segment downstream. This function must only be called by derived sub-classes, and only from the [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function, as the stream-lock needs to be held. This method also requires that an out caps has been configured, so[GstBase.BaseSrc.set\_caps](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fcaps) needs to have been called before.

The format for the _segment_ must be identical with the current format of the source, as configured with [GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

The format of _src_ must not be [Gst.Format.UNDEFINED](gstreamer/gstformat.html#GST%5FFORMAT%5FUNDEFINED) and the format should be configured via [GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat) before calling this method.

This is a variant of [GstBase.BaseSrc.new\_segment](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fnew%5Fsegment) sending the segment right away, which can be useful to ensure events ordering.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

a pointer to a [Gst.Segment](gstreamer/gstsegment.html#GstSegment)

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if sending of new segment succeeded.

**Since** : 1.24

---

#### _gst\_base\_src\_query\_latency_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_query_latency ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                            [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") * live,
                            [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") * min_latency,
                            [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") * max_latency)

Query the source for the latency parameters. _live_ will be [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) when _src_ is configured as a live source. _min\_latency_ and _max\_latency_ will be set to the difference between the running time and the timestamp of the first buffer.

This function is mostly used by subclasses.

**Parameters:**

__`src`_–

the source

__`live`_ ( \[out\]\[allow-none\])–

if the source is live

__`minlatency`_ ( \[out\]\[allow-none\])–

the min latency of the source

__`maxlatency`_ ( \[out\]\[allow-none\])–

the max latency of the source

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the query succeeded.

---

#### _GstBase.BaseSrc.prototype.query\_latency_ 


`function GstBase.BaseSrc.prototype.query_latency(): {
    // javascript wrapper for 'gst_base_src_query_latency'
}`

Query the source for the latency parameters. _live_ will be [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) when _src_ is configured as a live source. _min\_latency_ and _max\_latency_ will be set to the difference between the running time and the timestamp of the first buffer.

This function is mostly used by subclasses.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns a tuple made of:**

([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the query succeeded.

__`live`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the query succeeded.

__`minlatency`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the query succeeded.

__`maxlatency`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the query succeeded.

---

#### _GstBase.BaseSrc.query\_latency_ 


`def GstBase.BaseSrc.query_latency (self):
    #python wrapper for 'gst_base_src_query_latency'`

Query the source for the latency parameters. _live_ will be [True](https://docs.python.org/3/library/constants.html#True) when _src_ is configured as a live source. _min\_latency_ and _max\_latency_ will be set to the difference between the running time and the timestamp of the first buffer.

This function is mostly used by subclasses.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

**Returns a tuple made of:**

([bool](https://docs.python.org/3/library/functions.html#bool "bool") )–

[True](https://docs.python.org/3/library/constants.html#True) if the query succeeded.

__`live`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool") )–

[True](https://docs.python.org/3/library/constants.html#True) if the query succeeded.

__`minlatency`_ ([int](https://docs.python.org/3/library/functions.html#int "int") )–

[True](https://docs.python.org/3/library/constants.html#True) if the query succeeded.

__`maxlatency`_ ([int](https://docs.python.org/3/library/functions.html#int "int") )–

[True](https://docs.python.org/3/library/constants.html#True) if the query succeeded.

---

#### _gst\_base\_src\_set\_async_ 


gst_base_src_set_async ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                        [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") async)

Configure async behaviour in _src_, no state change will block. The open, close, start, stop, play and pause virtual methods will be executed in a different thread and are thus allowed to perform blocking operations. Any blocking operation should be unblocked with the unlock vmethod.

**Parameters:**

__`src`_–

base source instance

__`async`_–

new async mode

---

#### _GstBase.BaseSrc.prototype.set\_async_ 


`function GstBase.BaseSrc.prototype.set_async(async: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_async'
}`

Configure async behaviour in _src_, no state change will block. The open, close, start, stop, play and pause virtual methods will be executed in a different thread and are thus allowed to perform blocking operations. Any blocking operation should be unblocked with the unlock vmethod.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`async`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

new async mode

---

#### _GstBase.BaseSrc.set\_async_ 


`def GstBase.BaseSrc.set_async (self, async):
    #python wrapper for 'gst_base_src_set_async'`

Configure async behaviour in _src_, no state change will block. The open, close, start, stop, play and pause virtual methods will be executed in a different thread and are thus allowed to perform blocking operations. Any blocking operation should be unblocked with the unlock vmethod.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`async`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

new async mode

---

#### _gst\_base\_src\_set\_automatic\_eos_ 


gst_base_src_set_automatic_eos ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                                [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") automatic_eos)

If _automatic\_eos_ is [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS), _src_ will automatically go EOS if a buffer after the total size is returned. By default this is [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) but sources that can't return an authoritative size and only know that they're EOS when trying to read more should set this to [FALSE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#FALSE:CAPS).

When _src_ operates in [GST\_FORMAT\_TIME](gstreamer/gstformat.html#GST%5FFORMAT%5FTIME), [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc) will send an EOS when a buffer outside of the currently configured segment is pushed if_automatic\_eos_ is [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS). Since 1.16, if _automatic\_eos_ is [FALSE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#FALSE:CAPS) an EOS will be pushed only when the [create](base/gstbasesrc.html#GstBaseSrcClass::create) implementation returns [GST\_FLOW\_EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS).

**Parameters:**

__`src`_–

base source instance

__`automaticeos`_–

automatic eos

**Since** : 1.4

---

#### _GstBase.BaseSrc.prototype.set\_automatic\_eos_ 


`function GstBase.BaseSrc.prototype.set_automatic_eos(automatic_eos: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_automatic_eos'
}`

If _automatic\_eos_ is [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean), _src_ will automatically go EOS if a buffer after the total size is returned. By default this is [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) but sources that can't return an authoritative size and only know that they're EOS when trying to read more should set this to [false](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean).

When _src_ operates in [Gst.Format.TIME](gstreamer/gstformat.html#GST%5FFORMAT%5FTIME), [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) will send an EOS when a buffer outside of the currently configured segment is pushed if_automatic\_eos_ is [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean). Since 1.16, if _automatic\_eos_ is [false](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) an EOS will be pushed only when the [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) implementation returns [Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`automaticeos`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

automatic eos

**Since** : 1.4

---

#### _GstBase.BaseSrc.set\_automatic\_eos_ 


`def GstBase.BaseSrc.set_automatic_eos (self, automatic_eos):
    #python wrapper for 'gst_base_src_set_automatic_eos'`

If _automatic\_eos_ is [True](https://docs.python.org/3/library/constants.html#True), _src_ will automatically go EOS if a buffer after the total size is returned. By default this is [True](https://docs.python.org/3/library/constants.html#True) but sources that can't return an authoritative size and only know that they're EOS when trying to read more should set this to [False](https://docs.python.org/3/library/constants.html#False).

When _src_ operates in [Gst.Format.TIME](gstreamer/gstformat.html#GST%5FFORMAT%5FTIME), [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc) will send an EOS when a buffer outside of the currently configured segment is pushed if_automatic\_eos_ is [True](https://docs.python.org/3/library/constants.html#True). Since 1.16, if _automatic\_eos_ is [False](https://docs.python.org/3/library/constants.html#False) an EOS will be pushed only when the [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) implementation returns [Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`automaticeos`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

automatic eos

**Since** : 1.4

---

#### _gst\_base\_src\_set\_blocksize_ 


gst_base_src_set_blocksize ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                            [guint](https://docs.gtk.org/glib/types.html#guint "guint") blocksize)

Set the number of bytes that _src_ will push out with each buffer. When_blocksize_ is set to -1, a default length will be used.

**Parameters:**

__`src`_–

the source

__`blocksize`_–

the new blocksize in bytes

---

#### _GstBase.BaseSrc.prototype.set\_blocksize_ 


`function GstBase.BaseSrc.prototype.set_blocksize(blocksize: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_blocksize'
}`

Set the number of bytes that _src_ will push out with each buffer. When_blocksize_ is set to -1, a default length will be used.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

__`blocksize`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the new blocksize in bytes

---

#### _GstBase.BaseSrc.set\_blocksize_ 


`def GstBase.BaseSrc.set_blocksize (self, blocksize):
    #python wrapper for 'gst_base_src_set_blocksize'`

Set the number of bytes that _src_ will push out with each buffer. When_blocksize_ is set to -1, a default length will be used.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

__`blocksize`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the new blocksize in bytes

---

#### _gst\_base\_src\_set\_caps_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_base_src_set_caps ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                       [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") * caps)

Set new caps on the basesrc source pad.

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`caps`_ ( \[transfer: none\])–

a [GstCaps](gstreamer/gstcaps.html#GstCaps)

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the caps could be set

---

#### _GstBase.BaseSrc.prototype.set\_caps_ 


`function GstBase.BaseSrc.prototype.set_caps(caps: [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")): {
    // javascript wrapper for 'gst_base_src_set_caps'
}`

Set new caps on the basesrc source pad.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

a [Gst.Caps](gstreamer/gstcaps.html#GstCaps)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the caps could be set

---

#### _GstBase.BaseSrc.set\_caps_ 


`def GstBase.BaseSrc.set_caps (self, caps):
    #python wrapper for 'gst_base_src_set_caps'`

Set new caps on the basesrc source pad.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

a [Gst.Caps](gstreamer/gstcaps.html#GstCaps)

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if the caps could be set

---

#### _gst\_base\_src\_set\_do\_timestamp_ 


gst_base_src_set_do_timestamp ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                               [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") timestamp)

Configure _src_ to automatically timestamp outgoing buffers based on the current running\_time of the pipeline. This property is mostly useful for live sources.

**Parameters:**

__`src`_–

the source

__`timestamp`_–

enable or disable timestamping

---

#### _GstBase.BaseSrc.prototype.set\_do\_timestamp_ 


`function GstBase.BaseSrc.prototype.set_do_timestamp(timestamp: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_do_timestamp'
}`

Configure _src_ to automatically timestamp outgoing buffers based on the current running\_time of the pipeline. This property is mostly useful for live sources.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

__`timestamp`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

enable or disable timestamping

---

#### _GstBase.BaseSrc.set\_do\_timestamp_ 


`def GstBase.BaseSrc.set_do_timestamp (self, timestamp):
    #python wrapper for 'gst_base_src_set_do_timestamp'`

Configure _src_ to automatically timestamp outgoing buffers based on the current running\_time of the pipeline. This property is mostly useful for live sources.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the source

__`timestamp`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

enable or disable timestamping

---

#### _gst\_base\_src\_set\_dynamic\_size_ 


gst_base_src_set_dynamic_size ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                               [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") dynamic)

If not _dynamic_, size is only updated when needed, such as when trying to read past current tracked size. Otherwise, size is checked for upon each read.

**Parameters:**

__`src`_–

base source instance

__`dynamic`_–

new dynamic size mode

---

#### _GstBase.BaseSrc.prototype.set\_dynamic\_size_ 


`function GstBase.BaseSrc.prototype.set_dynamic_size(dynamic: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_dynamic_size'
}`

If not _dynamic_, size is only updated when needed, such as when trying to read past current tracked size. Otherwise, size is checked for upon each read.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`dynamic`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

new dynamic size mode

---

#### _GstBase.BaseSrc.set\_dynamic\_size_ 


`def GstBase.BaseSrc.set_dynamic_size (self, dynamic):
    #python wrapper for 'gst_base_src_set_dynamic_size'`

If not _dynamic_, size is only updated when needed, such as when trying to read past current tracked size. Otherwise, size is checked for upon each read.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`dynamic`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

new dynamic size mode

---

#### _gst\_base\_src\_set\_format_ 


gst_base_src_set_format ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                         [GstFormat](gstreamer/gstformat.html#GstFormat "GstFormat") format)

Sets the default format of the source. This will be the format used for sending SEGMENT events and for performing seeks.

If a format of GST\_FORMAT\_BYTES is set, the element will be able to operate in pull mode if the [is\_seekable](base/gstbasesrc.html#GstBaseSrcClass::is%5Fseekable) returns [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS).

This function must only be called in states < [GST\_STATE\_PAUSED](gstreamer/gstelement.html#GST%5FSTATE%5FPAUSED).

**Parameters:**

__`src`_–

base source instance

__`format`_–

the format to use

---

#### _GstBase.BaseSrc.prototype.set\_format_ 


`function GstBase.BaseSrc.prototype.set_format(format: [Gst.Format](gstreamer/gstformat.html#GstFormat "Gst.Format")): {
    // javascript wrapper for 'gst_base_src_set_format'
}`

Sets the default format of the source. This will be the format used for sending SEGMENT events and for performing seeks.

If a format of GST\_FORMAT\_BYTES is set, the element will be able to operate in pull mode if the [vfunc\_is\_seekable](base/gstbasesrc.html#GstBaseSrcClass::is%5Fseekable) returns [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean).

This function must only be called in states < [Gst.State.PAUSED](gstreamer/gstelement.html#GST%5FSTATE%5FPAUSED).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`format`_ ([Gst.Format](gstreamer/gstformat.html#GstFormat "Gst.Format"))–

the format to use

---

#### _GstBase.BaseSrc.set\_format_ 


`def GstBase.BaseSrc.set_format (self, format):
    #python wrapper for 'gst_base_src_set_format'`

Sets the default format of the source. This will be the format used for sending SEGMENT events and for performing seeks.

If a format of GST\_FORMAT\_BYTES is set, the element will be able to operate in pull mode if the [do\_is\_seekable](base/gstbasesrc.html#GstBaseSrcClass::is%5Fseekable) returns [True](https://docs.python.org/3/library/constants.html#True).

This function must only be called in states < [Gst.State.PAUSED](gstreamer/gstelement.html#GST%5FSTATE%5FPAUSED).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`format`_ ([Gst.Format](gstreamer/gstformat.html#GstFormat "Gst.Format"))–

the format to use

---

#### _gst\_base\_src\_set\_live_ 


gst_base_src_set_live ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                       [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") live)

If the element listens to a live source, _live_ should be set to [TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS).

A live source will not produce data in the PAUSED state and will therefore not be able to participate in the PREROLL phase of a pipeline. To signal this fact to the application and the pipeline, the state change return value of the live source will be GST\_STATE\_CHANGE\_NO\_PREROLL.

**Parameters:**

__`src`_–

base source instance

__`live`_–

new live-mode

---

#### _GstBase.BaseSrc.prototype.set\_live_ 


`function GstBase.BaseSrc.prototype.set_live(live: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_base_src_set_live'
}`

If the element listens to a live source, _live_ should be set to [true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean).

A live source will not produce data in the PAUSED state and will therefore not be able to participate in the PREROLL phase of a pipeline. To signal this fact to the application and the pipeline, the state change return value of the live source will be GST\_STATE\_CHANGE\_NO\_PREROLL.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`live`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

new live-mode

---

#### _GstBase.BaseSrc.set\_live_ 


`def GstBase.BaseSrc.set_live (self, live):
    #python wrapper for 'gst_base_src_set_live'`

If the element listens to a live source, _live_ should be set to [True](https://docs.python.org/3/library/constants.html#True).

A live source will not produce data in the PAUSED state and will therefore not be able to participate in the PREROLL phase of a pipeline. To signal this fact to the application and the pipeline, the state change return value of the live source will be GST\_STATE\_CHANGE\_NO\_PREROLL.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`live`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

new live-mode

---

#### _gst\_base\_src\_start\_complete_ 


gst_base_src_start_complete ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * basesrc,
                             [GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn") ret)

Complete an asynchronous start operation. When the subclass overrides the start method, it should call [gst\_base\_src\_start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete) when the start operation completes either from the same thread or from an asynchronous helper thread.

**Parameters:**

__`basesrc`_–

base source instance

__`ret`_–

a [GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn)

---

#### _GstBase.BaseSrc.prototype.start\_complete_ 


`function GstBase.BaseSrc.prototype.start_complete(ret: [Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn")): {
    // javascript wrapper for 'gst_base_src_start_complete'
}`

Complete an asynchronous start operation. When the subclass overrides the start method, it should call [GstBase.BaseSrc.prototype.start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete) when the start operation completes either from the same thread or from an asynchronous helper thread.

**Parameters:**

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`ret`_ ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

a [Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn)

---

#### _GstBase.BaseSrc.start\_complete_ 


`def GstBase.BaseSrc.start_complete (self, ret):
    #python wrapper for 'gst_base_src_start_complete'`

Complete an asynchronous start operation. When the subclass overrides the start method, it should call [GstBase.BaseSrc.start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete) when the start operation completes either from the same thread or from an asynchronous helper thread.

**Parameters:**

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

__`ret`_ ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

a [Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn)

---

#### _gst\_base\_src\_start\_wait_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_base_src_start_wait ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * basesrc)

Wait until the start operation completes.

**Parameters:**

__`basesrc`_–

base source instance

**Returns**–

a [GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn).

---

#### _GstBase.BaseSrc.prototype.start\_wait_ 


`function GstBase.BaseSrc.prototype.start_wait(): {
    // javascript wrapper for 'gst_base_src_start_wait'
}`

Wait until the start operation completes.

**Parameters:**

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

a [Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn).

---

#### _GstBase.BaseSrc.start\_wait_ 


`def GstBase.BaseSrc.start_wait (self):
    #python wrapper for 'gst_base_src_start_wait'`

Wait until the start operation completes.

**Parameters:**

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

base source instance

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

a [Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn).

---

#### _gst\_base\_src\_submit\_buffer\_list_ 


gst_base_src_submit_buffer_list ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                                 [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList "GstBufferList") * buffer_list)

Subclasses can call this from their create virtual method implementation to submit a buffer list to be pushed out later. This is useful in cases where the create function wants to produce multiple buffers to be pushed out in one go in form of a [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList), which can reduce overhead drastically, especially for packetised inputs (for data streams where the packetisation/chunking is not important it is usually more efficient to return larger buffers instead).

Subclasses that use this function from their create function must return[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) and no buffer from their create virtual method implementation. If a buffer is returned after a buffer list has also been submitted via this function the behaviour is undefined.

Subclasses must only call this function once per create function call and subclasses must only call this function when the source operates in push mode.

**Parameters:**

__`src`_–

a [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`bufferlist`_ ( \[transfer: full\])–

a [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList)

**Since** : 1.14

---

#### _GstBase.BaseSrc.prototype.submit\_buffer\_list_ 


`function GstBase.BaseSrc.prototype.submit_buffer_list(buffer_list: [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList")): {
    // javascript wrapper for 'gst_base_src_submit_buffer_list'
}`

Subclasses can call this from their create virtual method implementation to submit a buffer list to be pushed out later. This is useful in cases where the create function wants to produce multiple buffers to be pushed out in one go in form of a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList), which can reduce overhead drastically, especially for packetised inputs (for data streams where the packetisation/chunking is not important it is usually more efficient to return larger buffers instead).

Subclasses that use this function from their create function must return[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) and no buffer from their create virtual method implementation. If a buffer is returned after a buffer list has also been submitted via this function the behaviour is undefined.

Subclasses must only call this function once per create function call and subclasses must only call this function when the source operates in push mode.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList)

**Since** : 1.14

---

#### _GstBase.BaseSrc.submit\_buffer\_list_ 


`def GstBase.BaseSrc.submit_buffer_list (self, buffer_list):
    #python wrapper for 'gst_base_src_submit_buffer_list'`

Subclasses can call this from their create virtual method implementation to submit a buffer list to be pushed out later. This is useful in cases where the create function wants to produce multiple buffers to be pushed out in one go in form of a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList), which can reduce overhead drastically, especially for packetised inputs (for data streams where the packetisation/chunking is not important it is usually more efficient to return larger buffers instead).

Subclasses that use this function from their create function must return[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) and no buffer from their create virtual method implementation. If a buffer is returned after a buffer list has also been submitted via this function the behaviour is undefined.

Subclasses must only call this function once per create function call and subclasses must only call this function when the source operates in push mode.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

a [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc)

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList)

**Since** : 1.14

---

#### _gst\_base\_src\_wait\_playing_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_base_src_wait_playing ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

If the [create](base/gstbasesrc.html#GstBaseSrcClass::create) method performs its own synchronisation against the clock it must unblock when going from PLAYING to the PAUSED state and call this method before continuing to produce the remaining data.

This function will block until a state change to PLAYING happens (in which case this function returns [GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK)) or the processing must be stopped due to a state change to READY or a FLUSH event (in which case this function returns [GST\_FLOW\_FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING)).

**Parameters:**

__`src`_–

the src

**Returns**–

[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) if _src_ is PLAYING and processing can continue. Any other return value should be returned from the create vmethod.

---

#### _GstBase.BaseSrc.prototype.wait\_playing_ 


`function GstBase.BaseSrc.prototype.wait_playing(): {
    // javascript wrapper for 'gst_base_src_wait_playing'
}`

If the [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) method performs its own synchronisation against the clock it must unblock when going from PLAYING to the PAUSED state and call this method before continuing to produce the remaining data.

This function will block until a state change to PLAYING happens (in which case this function returns [Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK)) or the processing must be stopped due to a state change to READY or a FLUSH event (in which case this function returns [Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING)).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the src

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) if _src_ is PLAYING and processing can continue. Any other return value should be returned from the create vmethod.

---

#### _GstBase.BaseSrc.wait\_playing_ 


`def GstBase.BaseSrc.wait_playing (self):
    #python wrapper for 'gst_base_src_wait_playing'`

If the [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) method performs its own synchronisation against the clock it must unblock when going from PLAYING to the PAUSED state and call this method before continuing to produce the remaining data.

This function will block until a state change to PLAYING happens (in which case this function returns [Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK)) or the processing must be stopped due to a state change to READY or a FLUSH event (in which case this function returns [Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING)).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

the src

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) if _src_ is PLAYING and processing can continue. Any other return value should be returned from the create vmethod.

---

### Properties

#### _automatic-eos_ 


“automatic-eos” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

See [gst\_base\_src\_set\_automatic\_eos](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fautomatic%5Feos)

 Flags : Read / Write

**Since** : 1.24

---

#### _automatic-eos_ 


“automatic-eos” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

See [GstBase.BaseSrc.prototype.set\_automatic\_eos](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fautomatic%5Feos)

 Flags : Read / Write

**Since** : 1.24

---

#### _automatic\_eos_ 


“self.props.automatic_eos” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

See [GstBase.BaseSrc.set\_automatic\_eos](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fautomatic%5Feos)

 Flags : Read / Write

**Since** : 1.24

---

#### _blocksize_ 


“blocksize” [guint](https://docs.gtk.org/glib/types.html#guint "guint")

 Flags : Read / Write

---

#### _blocksize_ 


“blocksize” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

 Flags : Read / Write

---

#### _blocksize_ 


“self.props.blocksize” [int](https://docs.python.org/3/library/functions.html#int "int")

 Flags : Read / Write

---

#### _do-timestamp_ 


“do-timestamp” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

 Flags : Read / Write

---

#### _do-timestamp_ 


“do-timestamp” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

 Flags : Read / Write

---

#### _do\_timestamp_ 


“self.props.do_timestamp” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

 Flags : Read / Write

---

#### _num-buffers_ 


“num-buffers” [gint](https://docs.gtk.org/glib/types.html#gint "gint")

 Flags : Read / Write

---

#### _num-buffers_ 


“num-buffers” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

 Flags : Read / Write

---

#### _num\_buffers_ 


“self.props.num_buffers” [int](https://docs.python.org/3/library/functions.html#int "int")

 Flags : Read / Write

---

#### _typefind_ 


“typefind” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

 Flags : Read / Write

---

#### _typefind_ 


“typefind” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

 Flags : Read / Write

---

#### _typefind_ 


“self.props.typefind” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

 Flags : Read / Write

---

### Virtual Methods

#### _alloc_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
alloc ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
       [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset,
       [guint](https://docs.gtk.org/glib/types.html#guint "guint") size,
       [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") ** buf)

Ask the subclass to allocate a buffer with for offset and size. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_–

_No description available_ 

__`offset`_–

_No description available_ 

__`size`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_alloc_ 


`function vfunc_alloc(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), size: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript implementation of the 'alloc' virtual method
}`

Ask the subclass to allocate a buffer with for offset and size. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

__`size`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _do\_alloc_ 


`def do_alloc (src, offset, size):
    #python implementation of the 'alloc' virtual method`

Ask the subclass to allocate a buffer with for offset and size. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

__`size`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _create_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
create ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
        [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset,
        [guint](https://docs.gtk.org/glib/types.html#guint "guint") size,
        [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") ** buf)

Ask the subclass to create a buffer with offset and size. When the subclass returns GST\_FLOW\_OK, it MUST return a buffer of the requested size unless fewer bytes are available because an EOS condition is near. No buffer should be returned when the return value is different from GST\_FLOW\_OK. A return value of GST\_FLOW\_EOS signifies that the end of stream is reached. The default implementation will call[alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc) and then call [fill](base/gstbasesrc.html#GstBaseSrcClass::fill).

**Parameters:**

__`src`_–

_No description available_ 

__`offset`_–

_No description available_ 

__`size`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_create_ 


`function vfunc_create(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), size: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), buf: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'create' virtual method
}`

Ask the subclass to create a buffer with offset and size. When the subclass returns GST\_FLOW\_OK, it MUST return a buffer of the requested size unless fewer bytes are available because an EOS condition is near. No buffer should be returned when the return value is different from GST\_FLOW\_OK. A return value of GST\_FLOW\_EOS signifies that the end of stream is reached. The default implementation will call[vfunc\_alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc) and then call [vfunc\_fill](base/gstbasesrc.html#GstBaseSrcClass::fill).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

__`size`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _do\_create_ 


`def do_create (src, offset, size, buf):
    #python implementation of the 'create' virtual method`

Ask the subclass to create a buffer with offset and size. When the subclass returns GST\_FLOW\_OK, it MUST return a buffer of the requested size unless fewer bytes are available because an EOS condition is near. No buffer should be returned when the return value is different from GST\_FLOW\_OK. A return value of GST\_FLOW\_EOS signifies that the end of stream is reached. The default implementation will call[do\_alloc](base/gstbasesrc.html#GstBaseSrcClass::alloc) and then call [do\_fill](base/gstbasesrc.html#GstBaseSrcClass::fill).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

__`size`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _decide\_allocation_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
decide_allocation ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                   [GstQuery](gstreamer/gstquery.html#GstQuery "GstQuery") * query)

configure the allocation query

**Parameters:**

__`src`_–

_No description available_ 

__`query`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_decide\_allocation_ 


`function vfunc_decide_allocation(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), query: [Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query")): {
    // javascript implementation of the 'decide_allocation' virtual method
}`

configure the allocation query

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`query`_ ([Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_decide\_allocation_ 


`def do_decide_allocation (src, query):
    #python implementation of the 'decide_allocation' virtual method`

configure the allocation query

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`query`_ ([Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _do\_seek_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
do_seek ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
         [GstSegment](gstreamer/gstsegment.html#GstSegment "GstSegment") * segment)

Perform seeking on the resource to the indicated segment.

**Parameters:**

__`src`_–

_No description available_ 

__`segment`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_do\_seek_ 


`function vfunc_do_seek(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), segment: [Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")): {
    // javascript implementation of the 'do_seek' virtual method
}`

Perform seeking on the resource to the indicated segment.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_do\_seek_ 


`def do_do_seek (src, segment):
    #python implementation of the 'do_seek' virtual method`

Perform seeking on the resource to the indicated segment.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _event_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
event ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
       [GstEvent](gstreamer/gstevent.html#GstEvent "GstEvent") * event)

Override this to implement custom event handling.

**Parameters:**

__`src`_–

_No description available_ 

__`event`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_event_ 


`function vfunc_event(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), event: [Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event")): {
    // javascript implementation of the 'event' virtual method
}`

Override this to implement custom event handling.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`event`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_event_ 


`def do_event (src, event):
    #python implementation of the 'event' virtual method`

Override this to implement custom event handling.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`event`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _fill_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
fill ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
      [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset,
      [guint](https://docs.gtk.org/glib/types.html#guint "guint") size,
      [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") * buf)

Ask the subclass to fill the buffer with data for offset and size. The passed buffer is guaranteed to hold the requested amount of bytes.

**Parameters:**

__`src`_–

_No description available_ 

__`offset`_–

_No description available_ 

__`size`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_fill_ 


`function vfunc_fill(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), size: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), buf: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'fill' virtual method
}`

Ask the subclass to fill the buffer with data for offset and size. The passed buffer is guaranteed to hold the requested amount of bytes.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

__`size`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_fill_ 


`def do_fill (src, offset, size, buf):
    #python implementation of the 'fill' virtual method`

Ask the subclass to fill the buffer with data for offset and size. The passed buffer is guaranteed to hold the requested amount of bytes.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

__`size`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _fixate_ 


[GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") *
fixate ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
        [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") * caps)

Called during negotiation if caps need fixating. Implement instead of setting a fixate function on the source pad.

**Parameters:**

__`src`_–

_No description available_ 

__`caps`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_fixate_ 


`function vfunc_fixate(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), caps: [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")): {
    // javascript implementation of the 'fixate' virtual method
}`

Called during negotiation if caps need fixating. Implement instead of setting a fixate function on the source pad.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

---

#### _do\_fixate_ 


`def do_fixate (src, caps):
    #python implementation of the 'fixate' virtual method`

Called during negotiation if caps need fixating. Implement instead of setting a fixate function on the source pad.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

---

#### _get\_caps_ 


[GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") *
get_caps ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
          [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") * filter)

Called to get the caps to report

**Parameters:**

__`src`_–

_No description available_ 

__`filter`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_get\_caps_ 


`function vfunc_get_caps(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), filter: [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")): {
    // javascript implementation of the 'get_caps' virtual method
}`

Called to get the caps to report

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`filter`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

---

#### _do\_get\_caps_ 


`def do_get_caps (src, filter):
    #python implementation of the 'get_caps' virtual method`

Called to get the caps to report

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`filter`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

---

#### _get\_size_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
get_size ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
          [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") * size)

Return the total size of the resource, in the format set by[gst\_base\_src\_set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

**Parameters:**

__`src`_–

_No description available_ 

__`size`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_get\_size_ 


`function vfunc_get_size(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'get_size' virtual method
}`

Return the total size of the resource, in the format set by[GstBase.BaseSrc.prototype.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns a tuple made of:**

([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

_No description available_ 

__`size`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number") )–

_No description available_ 

---

#### _do\_get\_size_ 


`def do_get_size (src):
    #python implementation of the 'get_size' virtual method`

Return the total size of the resource, in the format set by[GstBase.BaseSrc.set\_format](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fset%5Fformat).

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns a tuple made of:**

([bool](https://docs.python.org/3/library/functions.html#bool "bool") )–

_No description available_ 

__`size`_ ([int](https://docs.python.org/3/library/functions.html#int "int") )–

_No description available_ 

---

#### _get\_times_ 


get_times ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
           [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") * buffer,
           [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") * start,
           [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") * end)

Given a buffer, return the start and stop time when it should be pushed out. The base class will sync on the clock using these times.

**Parameters:**

__`src`_–

_No description available_ 

__`buffer`_–

_No description available_ 

__`start`_–

_No description available_ 

__`end`_–

_No description available_ 

---

#### _vfunc\_get\_times_ 


`function vfunc_get_times(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), buffer: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'get_times' virtual method
}`

Given a buffer, return the start and stop time when it should be pushed out. The base class will sync on the clock using these times.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

---

#### _do\_get\_times_ 


`def do_get_times (src, buffer):
    #python implementation of the 'get_times' virtual method`

Given a buffer, return the start and stop time when it should be pushed out. The base class will sync on the clock using these times.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

---

#### _is\_seekable_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
is_seekable ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Check if the source can seek

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_is\_seekable_ 


`function vfunc_is_seekable(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'is_seekable' virtual method
}`

Check if the source can seek

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_is\_seekable_ 


`def do_is_seekable (src):
    #python implementation of the 'is_seekable' virtual method`

Check if the source can seek

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _negotiate_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
negotiate ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Negotiated the caps with the peer.

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_negotiate_ 


`function vfunc_negotiate(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'negotiate' virtual method
}`

Negotiated the caps with the peer.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_negotiate_ 


`def do_negotiate (src):
    #python implementation of the 'negotiate' virtual method`

Negotiated the caps with the peer.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _prepare\_seek\_segment_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
prepare_seek_segment ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
                      [GstEvent](gstreamer/gstevent.html#GstEvent "GstEvent") * seek,
                      [GstSegment](gstreamer/gstsegment.html#GstSegment "GstSegment") * segment)

Prepare the [GstSegment](gstreamer/gstsegment.html#GstSegment) that will be passed to the[do\_seek](base/gstbasesrc.html#GstBaseSrcClass::do%5Fseek) vmethod for executing a seek request. Sub-classes should override this if they support seeking in formats other than the configured native format. By default, it tries to convert the seek arguments to the configured native format and prepare a segment in that format.

**Parameters:**

__`src`_–

_No description available_ 

__`seek`_–

_No description available_ 

__`segment`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_prepare\_seek\_segment_ 


`function vfunc_prepare_seek_segment(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), seek: [Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event"), segment: [Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment")): {
    // javascript implementation of the 'prepare_seek_segment' virtual method
}`

Prepare the [Gst.Segment](gstreamer/gstsegment.html#GstSegment) that will be passed to the[vfunc\_do\_seek](base/gstbasesrc.html#GstBaseSrcClass::do%5Fseek) vmethod for executing a seek request. Sub-classes should override this if they support seeking in formats other than the configured native format. By default, it tries to convert the seek arguments to the configured native format and prepare a segment in that format.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`seek`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event"))–

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_prepare\_seek\_segment_ 


`def do_prepare_seek_segment (src, seek, segment):
    #python implementation of the 'prepare_seek_segment' virtual method`

Prepare the [Gst.Segment](gstreamer/gstsegment.html#GstSegment) that will be passed to the[do\_do\_seek](base/gstbasesrc.html#GstBaseSrcClass::do%5Fseek) vmethod for executing a seek request. Sub-classes should override this if they support seeking in formats other than the configured native format. By default, it tries to convert the seek arguments to the configured native format and prepare a segment in that format.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`seek`_ ([Gst.Event](gstreamer/gstevent.html#GstEvent "Gst.Event"))–

_No description available_ 

__`segment`_ ([Gst.Segment](gstreamer/gstsegment.html#GstSegment "Gst.Segment"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _query_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
query ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
       [GstQuery](gstreamer/gstquery.html#GstQuery "GstQuery") * query)

Handle a requested query.

**Parameters:**

__`src`_–

_No description available_ 

__`query`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_query_ 


`function vfunc_query(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), query: [Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query")): {
    // javascript implementation of the 'query' virtual method
}`

Handle a requested query.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`query`_ ([Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_query_ 


`def do_query (src, query):
    #python implementation of the 'query' virtual method`

Handle a requested query.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`query`_ ([Gst.Query](gstreamer/gstquery.html#GstQuery "Gst.Query"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _set\_caps_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
set_caps ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src,
          [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") * caps)

Notify subclass of changed output caps

**Parameters:**

__`src`_–

_No description available_ 

__`caps`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_set\_caps_ 


`function vfunc_set_caps(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"), caps: [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")): {
    // javascript implementation of the 'set_caps' virtual method
}`

Notify subclass of changed output caps

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_set\_caps_ 


`def do_set_caps (src, caps):
    #python implementation of the 'set_caps' virtual method`

Notify subclass of changed output caps

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _start_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
start ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Start processing. Subclasses should open resources and prepare to produce data. Implementation should call [gst\_base\_src\_start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete)when the operation completes, either from the current thread or any other thread that finishes the start operation asynchronously.

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_start_ 


`function vfunc_start(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'start' virtual method
}`

Start processing. Subclasses should open resources and prepare to produce data. Implementation should call [GstBase.BaseSrc.prototype.start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete)when the operation completes, either from the current thread or any other thread that finishes the start operation asynchronously.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_start_ 


`def do_start (src):
    #python implementation of the 'start' virtual method`

Start processing. Subclasses should open resources and prepare to produce data. Implementation should call [GstBase.BaseSrc.start\_complete](base/gstbasesrc.html#gst%5Fbase%5Fsrc%5Fstart%5Fcomplete)when the operation completes, either from the current thread or any other thread that finishes the start operation asynchronously.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _stop_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
stop ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Stop processing. Subclasses should use this to close resources.

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_stop_ 


`function vfunc_stop(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'stop' virtual method
}`

Stop processing. Subclasses should use this to close resources.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_stop_ 


`def do_stop (src):
    #python implementation of the 'stop' virtual method`

Stop processing. Subclasses should use this to close resources.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _unlock_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
unlock ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Unlock any pending access to the resource. Subclasses should unblock any blocked function ASAP. In particular, any `create()` function in progress should be unblocked and should return GST\_FLOW\_FLUSHING. Any future [create](base/gstbasesrc.html#GstBaseSrcClass::create) function call should also return GST\_FLOW\_FLUSHING until the [unlock\_stop](base/gstbasesrc.html#GstBaseSrcClass::unlock%5Fstop) function has been called.

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_unlock_ 


`function vfunc_unlock(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'unlock' virtual method
}`

Unlock any pending access to the resource. Subclasses should unblock any blocked function ASAP. In particular, any `create()` function in progress should be unblocked and should return GST\_FLOW\_FLUSHING. Any future [vfunc\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function call should also return GST\_FLOW\_FLUSHING until the [vfunc\_unlock\_stop](base/gstbasesrc.html#GstBaseSrcClass::unlock%5Fstop) function has been called.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_unlock_ 


`def do_unlock (src):
    #python implementation of the 'unlock' virtual method`

Unlock any pending access to the resource. Subclasses should unblock any blocked function ASAP. In particular, any `create()` function in progress should be unblocked and should return GST\_FLOW\_FLUSHING. Any future [do\_create](base/gstbasesrc.html#GstBaseSrcClass::create) function call should also return GST\_FLOW\_FLUSHING until the [do\_unlock\_stop](base/gstbasesrc.html#GstBaseSrcClass::unlock%5Fstop) function has been called.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

#### _unlock\_stop_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
unlock_stop ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc") * src)

Clear the previous unlock request. Subclasses should clear any state they set during [unlock](base/gstbasesrc.html#GstBaseSrcClass::unlock), such as clearing command queues.

**Parameters:**

__`src`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_unlock\_stop_ 


`function vfunc_unlock_stop(src: [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")): {
    // javascript implementation of the 'unlock_stop' virtual method
}`

Clear the previous unlock request. Subclasses should clear any state they set during [vfunc\_unlock](base/gstbasesrc.html#GstBaseSrcClass::unlock), such as clearing command queues.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_unlock\_stop_ 


`def do_unlock_stop (src):
    #python implementation of the 'unlock_stop' virtual method`

Clear the previous unlock request. Subclasses should clear any state they set during [do\_unlock](base/gstbasesrc.html#GstBaseSrcClass::unlock), such as clearing command queues.

**Parameters:**

__`src`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

## Function Macros

### _GST\_BASE\_SRC\_CAST_ 


#define GST_BASE_SRC_CAST(obj)          ((GstBaseSrc *)(obj))

---

### _GST\_BASE\_SRC\_IS\_STARTED_ 


#define GST_BASE_SRC_IS_STARTED(obj)  GST_OBJECT_FLAG_IS_SET ((obj), GST_BASE_SRC_FLAG_STARTED)

---

### _GST\_BASE\_SRC\_IS\_STARTING_ 


#define GST_BASE_SRC_IS_STARTING(obj) GST_OBJECT_FLAG_IS_SET ((obj), GST_BASE_SRC_FLAG_STARTING)

---

### _GST\_BASE\_SRC\_PAD_ 


#define GST_BASE_SRC_PAD(obj)                 (GST_BASE_SRC_CAST (obj)->srcpad)

Gives the pointer to the [GstPad](gstreamer/gstpad.html#GstPad) object of the element.

**Parameters:**

__`obj`_–

base source instance

---

## Enumerations

### _GstBaseSrcFlags_ 

The [GstElement](gstreamer/gstelement.html#GstElement) flags that a basesrc element may have.

##### Members

__`GSTBASESRCFLAGSTARTING`_ (16384) –

has source is starting

__`GSTBASESRCFLAGSTARTED`_ (32768) –

has source been started

__`GSTBASESRCFLAGLAST`_ (1048576) –

offset to define more flags

---

### _GstBase.BaseSrcFlags_ 

The [Gst.Element](gstreamer/gstelement.html#GstElement) flags that a basesrc element may have.

##### Members

__`GstBase.BaseSrcFlags.STARTING`_ (16384) –

has source is starting

__`GstBase.BaseSrcFlags.STARTED`_ (32768) –

has source been started

__`GstBase.BaseSrcFlags.LAST`_ (1048576) –

offset to define more flags

---

### _GstBase.BaseSrcFlags_ 

The [Gst.Element](gstreamer/gstelement.html#GstElement) flags that a basesrc element may have.

##### Members

__`GstBase.BaseSrcFlags.STARTING`_ (16384) –

has source is starting

__`GstBase.BaseSrcFlags.STARTED`_ (32768) –

has source been started

__`GstBase.BaseSrcFlags.LAST`_ (1048576) –

offset to define more flags

---

The results of the search are