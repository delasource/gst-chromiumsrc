Title: GstAppSrc

URL Source: https://gstreamer.freedesktop.org/documentation/applib/gstappsrc.html

Markdown Content:
# GstAppSrc

The appsrc element can be used by applications to insert data into a GStreamer pipeline. Unlike most GStreamer elements, appsrc provides external API functions.

appsrc can be used by linking with the libgstapp library to access the methods directly or by using the appsrc action signals.

Before operating appsrc, the caps property must be set to fixed caps describing the format of the data that will be pushed with appsrc. An exception to this is when pushing buffers with unknown caps, in which case no caps should be set. This is typically true of file-like sources that push raw byte buffers. If you don't want to explicitly set the caps, you can use gst\_app\_src\_push\_sample. This method gets the caps associated with the sample and sets them on the appsrc replacing any previously set caps (if different from sample's caps).

The main way of handing data to the appsrc element is by calling the[gst\_app\_src\_push\_buffer](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fpush%5Fbuffer) method or by emitting the push-buffer action signal. This will put the buffer onto a queue from which appsrc will read from in its streaming thread. It is important to note that data transport will not happen from the thread that performed the push-buffer call.

The "max-bytes", "max-buffers" and "max-time" properties control how much data can be queued in appsrc before appsrc considers the queue full. A filled internal queue will always signal the "enough-data" signal, which signals the application that it should stop pushing data into appsrc. The "block" property will cause appsrc to block the push-buffer method until free data becomes available again.

When the internal queue is running out of data, the "need-data" signal is emitted, which signals the application that it should start pushing more data into appsrc.

In addition to the "need-data" and "enough-data" signals, appsrc can emit the "seek-data" signal when the "stream-mode" property is set to "seekable" or "random-access". The signal argument will contain the new desired position in the stream expressed in the unit set with the "format" property. After receiving the seek-data signal, the application should push-buffers from the new position.

These signals allow the application to operate the appsrc in two different ways:

The push mode, in which the application repeatedly calls the push-buffer/push-sample method with a new buffer/sample. Optionally, the queue size in the appsrc can be controlled with the enough-data and need-data signals by respectively stopping/starting the push-buffer/push-sample calls. This is a typical mode of operation for the stream-type "stream" and "seekable". Use this mode when implementing various network protocols or hardware devices.

The pull mode, in which the need-data signal triggers the next push-buffer call. This mode is typically used in the "random-access" stream-type. Use this mode for file access or other randomly accessible sources. In this mode, a buffer of exactly the amount of bytes given by the need-data signal should be pushed into appsrc.

In all modes, the size property on appsrc should contain the total stream size in bytes. Setting this property is mandatory in the random-access mode. For the stream and seekable modes, setting this property is optional but recommended.

When the application has finished pushing data into appsrc, it should call[gst\_app\_src\_end\_of\_stream](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fend%5Fof%5Fstream) or emit the end-of-stream action signal. After this call, no more buffers can be pushed into appsrc until a flushing seek occurs or the state of the appsrc has gone through READY.

## _GstAppSrc_ 


[GObject](https://docs.gtk.org/gobject/class.Object.html "GObject")
    ╰──[GInitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GInitiallyUnowned")
        ╰──[GstObject](gstreamer/gstobject.html#GstObject "GstObject")
            ╰──[GstElement](gstreamer/gstelement.html#GstElement "GstElement")
                ╰──[GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc")
                    ╰──GstAppSrc

### Members

__`basesrc`_ ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc")) –

_No description available_ 

---

### Class structure

#### _GstAppSrcClass_ 

##### Fields

__`basesrcclass`_ (<GstBaseSrcClass>) –

_No description available_ 

---

###  GstApp.AppSrcClass

##### Attributes

__`basesrcclass`_ ([GstBase.BaseSrcClass](GstBaseSrcClass "GstBase.BaseSrcClass")) –

_No description available_ 

---

###  GstApp.AppSrcClass

##### Attributes

__`basesrcclass`_ ([GstBase.BaseSrcClass](GstBaseSrcClass "GstBase.BaseSrcClass")) –

_No description available_ 

---

## _GstApp.AppSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")
                    ╰──GstApp.AppSrc

### Members

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")) –

_No description available_ 

---

## _GstApp.AppSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")
                    ╰──GstApp.AppSrc

### Members

__`basesrc`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")) –

_No description available_ 

---

### Methods

#### _gst\_app\_src\_end\_of\_stream_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_app_src_end_of_stream ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Indicates to the appsrc element that the last buffer queued in the element is the last buffer of the stream.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the EOS was successfully queued.[GST\_FLOW\_FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.

---

#### _GstApp.AppSrc.prototype.end\_of\_stream_ 


`function GstApp.AppSrc.prototype.end_of_stream(): {
    // javascript wrapper for 'gst_app_src_end_of_stream'
}`

Indicates to the appsrc element that the last buffer queued in the element is the last buffer of the stream.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the EOS was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.

---

#### _GstApp.AppSrc.end\_of\_stream_ 


`def GstApp.AppSrc.end_of_stream (self):
    #python wrapper for 'gst_app_src_end_of_stream'`

Indicates to the appsrc element that the last buffer queued in the element is the last buffer of the stream.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the EOS was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.

---

#### _gst\_app\_src\_get\_caps_ 


[GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") *
gst_app_src_get_caps ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the configured caps on _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ( \[nullable\]\[transfer: full\])–

the [GstCaps](gstreamer/gstcaps.html#GstCaps) produced by the source. [gst\_caps\_unref](gstreamer/gstcaps.html#gst%5Fcaps%5Funref) after usage.

---

#### _GstApp.AppSrc.prototype.get\_caps_ 


`function GstApp.AppSrc.prototype.get_caps(): {
    // javascript wrapper for 'gst_app_src_get_caps'
}`

Get the configured caps on _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

the [Gst.Caps](gstreamer/gstcaps.html#GstCaps) produced by the source. [gst\_caps\_unref (not introspectable)](gstreamer/gstcaps.html#gst%5Fcaps%5Funref) after usage.

---

#### _GstApp.AppSrc.get\_caps_ 


`def GstApp.AppSrc.get_caps (self):
    #python wrapper for 'gst_app_src_get_caps'`

Get the configured caps on _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

the [Gst.Caps](gstreamer/gstcaps.html#GstCaps) produced by the source. [gst\_caps\_unref (not introspectable)](gstreamer/gstcaps.html#gst%5Fcaps%5Funref) after usage.

---

#### _gst\_app\_src\_get\_current\_level\_buffers_ 


[guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")
gst_app_src_get_current_level_buffers ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the number of currently queued buffers inside _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The number of currently queued buffers.

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.get\_current\_level\_buffers_ 


`function GstApp.AppSrc.prototype.get_current_level_buffers(): {
    // javascript wrapper for 'gst_app_src_get_current_level_buffers'
}`

Get the number of currently queued buffers inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The number of currently queued buffers.

**Since** : 1.20

---

#### _GstApp.AppSrc.get\_current\_level\_buffers_ 


`def GstApp.AppSrc.get_current_level_buffers (self):
    #python wrapper for 'gst_app_src_get_current_level_buffers'`

Get the number of currently queued buffers inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The number of currently queued buffers.

**Since** : 1.20

---

#### _gst\_app\_src\_get\_current\_level\_bytes_ 


[guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")
gst_app_src_get_current_level_bytes ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the number of currently queued bytes inside _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The number of currently queued bytes.

**Since** : 1.2

---

#### _GstApp.AppSrc.prototype.get\_current\_level\_bytes_ 


`function GstApp.AppSrc.prototype.get_current_level_bytes(): {
    // javascript wrapper for 'gst_app_src_get_current_level_bytes'
}`

Get the number of currently queued bytes inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The number of currently queued bytes.

**Since** : 1.2

---

#### _GstApp.AppSrc.get\_current\_level\_bytes_ 


`def GstApp.AppSrc.get_current_level_bytes (self):
    #python wrapper for 'gst_app_src_get_current_level_bytes'`

Get the number of currently queued bytes inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The number of currently queued bytes.

**Since** : 1.2

---

#### _gst\_app\_src\_get\_current\_level\_time_ 


[GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime")
gst_app_src_get_current_level_time ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the amount of currently queued time inside _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The amount of currently queued time.

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.get\_current\_level\_time_ 


`function GstApp.AppSrc.prototype.get_current_level_time(): {
    // javascript wrapper for 'gst_app_src_get_current_level_time'
}`

Get the amount of currently queued time inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The amount of currently queued time.

**Since** : 1.20

---

#### _GstApp.AppSrc.get\_current\_level\_time_ 


`def GstApp.AppSrc.get_current_level_time (self):
    #python wrapper for 'gst_app_src_get_current_level_time'`

Get the amount of currently queued time inside _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The amount of currently queued time.

**Since** : 1.20

---

#### _gst\_app\_src\_get\_duration_ 


[GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime")
gst_app_src_get_duration ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

the duration of the stream previously set with [gst\_app\_src\_set\_duration](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fduration);

**Since** : 1.10

---

#### _GstApp.AppSrc.prototype.get\_duration_ 


`function GstApp.AppSrc.prototype.get_duration(): {
    // javascript wrapper for 'gst_app_src_get_duration'
}`

Get the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the duration of the stream previously set with [GstApp.AppSrc.prototype.set\_duration](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fduration);

**Since** : 1.10

---

#### _GstApp.AppSrc.get\_duration_ 


`def GstApp.AppSrc.get_duration (self):
    #python wrapper for 'gst_app_src_get_duration'`

Get the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the duration of the stream previously set with [GstApp.AppSrc.set\_duration](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fduration);

**Since** : 1.10

---

#### _gst\_app\_src\_get\_emit\_signals_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
gst_app_src_get_emit_signals ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Check if appsrc will emit the "new-preroll" and "new-buffer" signals.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if _appsrc_ is emitting the "new-preroll" and "new-buffer" signals.

---

#### _GstApp.AppSrc.prototype.get\_emit\_signals_ 


`function GstApp.AppSrc.prototype.get_emit_signals(): {
    // javascript wrapper for 'gst_app_src_get_emit_signals'
}`

Check if appsrc will emit the "new-preroll" and "new-buffer" signals.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if _appsrc_ is emitting the "new-preroll" and "new-buffer" signals.

---

#### _GstApp.AppSrc.get\_emit\_signals_ 


`def GstApp.AppSrc.get_emit_signals (self):
    #python wrapper for 'gst_app_src_get_emit_signals'`

Check if appsrc will emit the "new-preroll" and "new-buffer" signals.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if _appsrc_ is emitting the "new-preroll" and "new-buffer" signals.

---

#### _gst\_app\_src\_get\_latency_ 


gst_app_src_get_latency ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                         [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") * min,
                         [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") * max)

Retrieve the min and max latencies in _min_ and _max_ respectively.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`min`_ ( \[out\])–

the min latency

__`max`_ ( \[out\])–

the max latency

---

#### _GstApp.AppSrc.prototype.get\_latency_ 


`function GstApp.AppSrc.prototype.get_latency(): {
    // javascript wrapper for 'gst_app_src_get_latency'
}`

Retrieve the min and max latencies in _min_ and _max_ respectively.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

---

#### _GstApp.AppSrc.get\_latency_ 


`def GstApp.AppSrc.get_latency (self):
    #python wrapper for 'gst_app_src_get_latency'`

Retrieve the min and max latencies in _min_ and _max_ respectively.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

---

#### _gst\_app\_src\_get\_leaky\_type_ 


[GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstAppLeakyType")
gst_app_src_get_leaky_type ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Returns the currently set [GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType). See [gst\_app\_src\_set\_leaky\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fleaky%5Ftype)for more details.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The currently set [GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType).

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.get\_leaky\_type_ 


`function GstApp.AppSrc.prototype.get_leaky_type(): {
    // javascript wrapper for 'gst_app_src_get_leaky_type'
}`

Returns the currently set [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType). See [GstApp.AppSrc.prototype.set\_leaky\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fleaky%5Ftype)for more details.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType"))–

The currently set [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType).

**Since** : 1.20

---

#### _GstApp.AppSrc.get\_leaky\_type_ 


`def GstApp.AppSrc.get_leaky_type (self):
    #python wrapper for 'gst_app_src_get_leaky_type'`

Returns the currently set [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType). See [GstApp.AppSrc.set\_leaky\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fleaky%5Ftype)for more details.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType"))–

The currently set [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType).

**Since** : 1.20

---

#### _gst\_app\_src\_get\_max\_buffers_ 


[guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")
gst_app_src_get_max_buffers ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the maximum amount of buffers that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The maximum amount of buffers that can be queued.

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.get\_max\_buffers_ 


`function GstApp.AppSrc.prototype.get_max_buffers(): {
    // javascript wrapper for 'gst_app_src_get_max_buffers'
}`

Get the maximum amount of buffers that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The maximum amount of buffers that can be queued.

**Since** : 1.20

---

#### _GstApp.AppSrc.get\_max\_buffers_ 


`def GstApp.AppSrc.get_max_buffers (self):
    #python wrapper for 'gst_app_src_get_max_buffers'`

Get the maximum amount of buffers that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The maximum amount of buffers that can be queued.

**Since** : 1.20

---

#### _gst\_app\_src\_get\_max\_bytes_ 


[guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")
gst_app_src_get_max_bytes ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the maximum amount of bytes that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The maximum amount of bytes that can be queued.

---

#### _GstApp.AppSrc.prototype.get\_max\_bytes_ 


`function GstApp.AppSrc.prototype.get_max_bytes(): {
    // javascript wrapper for 'gst_app_src_get_max_bytes'
}`

Get the maximum amount of bytes that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The maximum amount of bytes that can be queued.

---

#### _GstApp.AppSrc.get\_max\_bytes_ 


`def GstApp.AppSrc.get_max_bytes (self):
    #python wrapper for 'gst_app_src_get_max_bytes'`

Get the maximum amount of bytes that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The maximum amount of bytes that can be queued.

---

#### _gst\_app\_src\_get\_max\_time_ 


[GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime")
gst_app_src_get_max_time ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the maximum amount of time that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

The maximum amount of time that can be queued.

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.get\_max\_time_ 


`function GstApp.AppSrc.prototype.get_max_time(): {
    // javascript wrapper for 'gst_app_src_get_max_time'
}`

Get the maximum amount of time that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

The maximum amount of time that can be queued.

**Since** : 1.20

---

#### _GstApp.AppSrc.get\_max\_time_ 


`def GstApp.AppSrc.get_max_time (self):
    #python wrapper for 'gst_app_src_get_max_time'`

Get the maximum amount of time that can be queued in _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

The maximum amount of time that can be queued.

**Since** : 1.20

---

#### _gst\_app\_src\_get\_size_ 


[gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64")
gst_app_src_get_size ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

the size of the stream previously set with [gst\_app\_src\_set\_size](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsize);

---

#### _GstApp.AppSrc.prototype.get\_size_ 


`function GstApp.AppSrc.prototype.get_size(): {
    // javascript wrapper for 'gst_app_src_get_size'
}`

Get the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the size of the stream previously set with [GstApp.AppSrc.prototype.set\_size](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsize);

---

#### _GstApp.AppSrc.get\_size_ 


`def GstApp.AppSrc.get_size (self):
    #python wrapper for 'gst_app_src_get_size'`

Get the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the size of the stream previously set with [GstApp.AppSrc.set\_size](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsize);

---

#### _gst\_app\_src\_get\_stream\_type_ 


[GstAppStreamType](applib/gstappsrc.html#GstAppStreamType "GstAppStreamType")
gst_app_src_get_stream_type ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

Get the stream type. Control the stream type of _appsrc_with [gst\_app\_src\_set\_stream\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fstream%5Ftype).

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns**–

the stream type.

---

#### _GstApp.AppSrc.prototype.get\_stream\_type_ 


`function GstApp.AppSrc.prototype.get_stream_type(): {
    // javascript wrapper for 'gst_app_src_get_stream_type'
}`

Get the stream type. Control the stream type of _appsrc_with [GstApp.AppSrc.prototype.set\_stream\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fstream%5Ftype).

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType"))–

the stream type.

---

#### _GstApp.AppSrc.get\_stream\_type_ 


`def GstApp.AppSrc.get_stream_type (self):
    #python wrapper for 'gst_app_src_get_stream_type'`

Get the stream type. Control the stream type of _appsrc_with [GstApp.AppSrc.set\_stream\_type](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fstream%5Ftype).

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

**Returns** ([GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType"))–

the stream type.

---

#### _gst\_app\_src\_push\_buffer_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_app_src_push_buffer ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                         [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") * buffer)

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad. This function takes ownership of the buffer.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`buffer`_ ( \[transfer: full\])–

a [GstBuffer](gstreamer/gstbuffer.html#GstBuffer) to push

**Returns**–

[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[GST\_FLOW\_FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[GST\_FLOW\_EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

---

#### _GstApp.AppSrc.prototype.push\_buffer_ 


`function GstApp.AppSrc.prototype.push_buffer(buffer: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript wrapper for 'gst_app_src_push_buffer'
}`

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad. This function takes ownership of the buffer.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

a [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer) to push

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

---

#### _GstApp.AppSrc.push\_buffer_ 


`def GstApp.AppSrc.push_buffer (self, buffer):
    #python wrapper for 'gst_app_src_push_buffer'`

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad. This function takes ownership of the buffer.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

a [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer) to push

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

---

#### _gst\_app\_src\_push\_buffer\_list_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_app_src_push_buffer_list ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                              [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList "GstBufferList") * buffer_list)

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad. This function takes ownership of _buffer\_list_.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`bufferlist`_ ( \[transfer: full\])–

a [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList) to push

**Returns**–

[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer list was successfully queued.[GST\_FLOW\_FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[GST\_FLOW\_EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.14

---

#### _GstApp.AppSrc.prototype.push\_buffer\_list_ 


`function GstApp.AppSrc.prototype.push_buffer_list(buffer_list: [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList")): {
    // javascript wrapper for 'gst_app_src_push_buffer_list'
}`

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad. This function takes ownership of _buffer\_list_.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList) to push

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer list was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.14

---

#### _GstApp.AppSrc.push\_buffer\_list_ 


`def GstApp.AppSrc.push_buffer_list (self, buffer_list):
    #python wrapper for 'gst_app_src_push_buffer_list'`

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad. This function takes ownership of _buffer\_list_.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList) to push

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer list was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.14

---

#### _gst\_app\_src\_push\_sample_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
gst_app_src_push_sample ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                         [GstSample](gstreamer/gstsample.html#GstSample "GstSample") * sample)

Extract a buffer from the provided sample and adds it to the queue of buffers that the appsrc element will push to its source pad. Any previous caps that were set on appsrc will be replaced by the caps associated with the sample if not equal.

This function does not take ownership of the sample so the sample needs to be unreffed after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`sample`_ ( \[transfer: none\])–

a [GstSample](gstreamer/gstsample.html#GstSample) from which buffer and caps may be extracted

**Returns**–

[GST\_FLOW\_OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[GST\_FLOW\_FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[GST\_FLOW\_EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.6

---

#### _GstApp.AppSrc.prototype.push\_sample_ 


`function GstApp.AppSrc.prototype.push_sample(sample: [Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample")): {
    // javascript wrapper for 'gst_app_src_push_sample'
}`

Extract a buffer from the provided sample and adds it to the queue of buffers that the appsrc element will push to its source pad. Any previous caps that were set on appsrc will be replaced by the caps associated with the sample if not equal.

This function does not take ownership of the sample so the sample needs to be unreffed after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

a [Gst.Sample](gstreamer/gstsample.html#GstSample) from which buffer and caps may be extracted

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.6

---

#### _GstApp.AppSrc.push\_sample_ 


`def GstApp.AppSrc.push_sample (self, sample):
    #python wrapper for 'gst_app_src_push_sample'`

Extract a buffer from the provided sample and adds it to the queue of buffers that the appsrc element will push to its source pad. Any previous caps that were set on appsrc will be replaced by the caps associated with the sample if not equal.

This function does not take ownership of the sample so the sample needs to be unreffed after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

a [Gst.Sample](gstreamer/gstsample.html#GstSample) from which buffer and caps may be extracted

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

[Gst.FlowReturn.OK](gstreamer/gstpad.html#GST%5FFLOW%5FOK) when the buffer was successfully queued.[Gst.FlowReturn.FLUSHING](gstreamer/gstpad.html#GST%5FFLOW%5FFLUSHING) when _appsrc_ is not PAUSED or PLAYING.[Gst.FlowReturn.EOS](gstreamer/gstpad.html#GST%5FFLOW%5FEOS) when EOS occurred.

**Since** : 1.6

---

#### _gst\_app\_src\_set\_callbacks_ 


gst_app_src_set_callbacks ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                           [GstAppSrcCallbacks](applib/gstappsrc.html#GstAppSrcCallbacks "GstAppSrcCallbacks") * callbacks,
                           [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data,
                           [GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html "GDestroyNotify") notify)

Set callbacks which will be executed when data is needed, enough data has been collected or when a seek should be performed. This is an alternative to using the signals, it has lower overhead and is thus less expensive, but also less flexible.

If callbacks are installed, no signals will be emitted for performance reasons.

Before 1.16.3 it was not possible to change the callbacks in a thread-safe way.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`callbacks`_–

the callbacks

__`userdata`_–

a user\_data argument for the callbacks

__`notify`_–

a destroy notify function

---

#### _gst\_app\_src\_set\_caps_ 


gst_app_src_set_caps ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                      const [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") * caps)

Set the capabilities on the appsrc element. This function takes a copy of the caps structure. After calling this method, the source will only produce caps that match _caps_. _caps_ must be fixed and the caps on the buffers must match the caps or left NULL.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`caps`_ ( \[nullable\])–

caps to set

---

#### _GstApp.AppSrc.prototype.set\_caps_ 


`function GstApp.AppSrc.prototype.set_caps(caps: [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")): {
    // javascript wrapper for 'gst_app_src_set_caps'
}`

Set the capabilities on the appsrc element. This function takes a copy of the caps structure. After calling this method, the source will only produce caps that match _caps_. _caps_ must be fixed and the caps on the buffers must match the caps or left NULL.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

caps to set

---

#### _GstApp.AppSrc.set\_caps_ 


`def GstApp.AppSrc.set_caps (self, caps):
    #python wrapper for 'gst_app_src_set_caps'`

Set the capabilities on the appsrc element. This function takes a copy of the caps structure. After calling this method, the source will only produce caps that match _caps_. _caps_ must be fixed and the caps on the buffers must match the caps or left NULL.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`caps`_ ([Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps"))–

caps to set

---

#### _gst\_app\_src\_set\_duration_ 


gst_app_src_set_duration ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                          [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") duration)

Set the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`duration`_–

the duration to set

**Since** : 1.10

---

#### _GstApp.AppSrc.prototype.set\_duration_ 


`function GstApp.AppSrc.prototype.set_duration(duration: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_duration'
}`

Set the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`duration`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the duration to set

**Since** : 1.10

---

#### _GstApp.AppSrc.set\_duration_ 


`def GstApp.AppSrc.set_duration (self, duration):
    #python wrapper for 'gst_app_src_set_duration'`

Set the duration of the stream in nanoseconds. A value of GST\_CLOCK\_TIME\_NONE means that the duration is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`duration`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the duration to set

**Since** : 1.10

---

#### _gst\_app\_src\_set\_emit\_signals_ 


gst_app_src_set_emit_signals ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                              [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean") emit)

Make appsrc emit the "new-preroll" and "new-buffer" signals. This option is by default disabled because signal emission is expensive and unneeded when the application prefers to operate in pull mode.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`emit`_–

the new state

---

#### _GstApp.AppSrc.prototype.set\_emit\_signals_ 


`function GstApp.AppSrc.prototype.set_emit_signals(emit: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_emit_signals'
}`

Make appsrc emit the "new-preroll" and "new-buffer" signals. This option is by default disabled because signal emission is expensive and unneeded when the application prefers to operate in pull mode.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`emit`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the new state

---

#### _GstApp.AppSrc.set\_emit\_signals_ 


`def GstApp.AppSrc.set_emit_signals (self, emit):
    #python wrapper for 'gst_app_src_set_emit_signals'`

Make appsrc emit the "new-preroll" and "new-buffer" signals. This option is by default disabled because signal emission is expensive and unneeded when the application prefers to operate in pull mode.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`emit`_ ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

the new state

---

#### _gst\_app\_src\_set\_latency_ 


gst_app_src_set_latency ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                         [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") min,
                         [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") max)

Configure the _min_ and _max_ latency in _src_. If _min_ is set to -1, the default latency calculations for pseudo-live sources will be used.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`min`_–

the min latency

__`max`_–

the max latency

---

#### _GstApp.AppSrc.prototype.set\_latency_ 


`function GstApp.AppSrc.prototype.set_latency(min: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), max: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_latency'
}`

Configure the _min_ and _max_ latency in _src_. If _min_ is set to -1, the default latency calculations for pseudo-live sources will be used.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`min`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the min latency

__`max`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the max latency

---

#### _GstApp.AppSrc.set\_latency_ 


`def GstApp.AppSrc.set_latency (self, min, max):
    #python wrapper for 'gst_app_src_set_latency'`

Configure the _min_ and _max_ latency in _src_. If _min_ is set to -1, the default latency calculations for pseudo-live sources will be used.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`min`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the min latency

__`max`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the max latency

---

#### _gst\_app\_src\_set\_leaky\_type_ 


gst_app_src_set_leaky_type ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                            [GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstAppLeakyType") leaky)

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`leaky`_–

the [GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType)

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.set\_leaky\_type_ 


`function GstApp.AppSrc.prototype.set_leaky_type(leaky: [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType")): {
    // javascript wrapper for 'gst_app_src_set_leaky_type'
}`

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`leaky`_ ([GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType"))–

the [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType)

**Since** : 1.20

---

#### _GstApp.AppSrc.set\_leaky\_type_ 


`def GstApp.AppSrc.set_leaky_type (self, leaky):
    #python wrapper for 'gst_app_src_set_leaky_type'`

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`leaky`_ ([GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType"))–

the [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType)

**Since** : 1.20

---

#### _gst\_app\_src\_set\_max\_buffers_ 


gst_app_src_set_max_buffers ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                             [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") max)

Set the maximum amount of buffers that can be queued in _appsrc_. After the maximum amount of buffers are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_–

the maximum number of buffers to queue

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.set\_max\_buffers_ 


`function GstApp.AppSrc.prototype.set_max_buffers(max: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_max_buffers'
}`

Set the maximum amount of buffers that can be queued in _appsrc_. After the maximum amount of buffers are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the maximum number of buffers to queue

**Since** : 1.20

---

#### _GstApp.AppSrc.set\_max\_buffers_ 


`def GstApp.AppSrc.set_max_buffers (self, max):
    #python wrapper for 'gst_app_src_set_max_buffers'`

Set the maximum amount of buffers that can be queued in _appsrc_. After the maximum amount of buffers are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the maximum number of buffers to queue

**Since** : 1.20

---

#### _gst\_app\_src\_set\_max\_bytes_ 


gst_app_src_set_max_bytes ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                           [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") max)

Set the maximum amount of bytes that can be queued in _appsrc_. After the maximum amount of bytes are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_–

the maximum number of bytes to queue

---

#### _GstApp.AppSrc.prototype.set\_max\_bytes_ 


`function GstApp.AppSrc.prototype.set_max_bytes(max: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_max_bytes'
}`

Set the maximum amount of bytes that can be queued in _appsrc_. After the maximum amount of bytes are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the maximum number of bytes to queue

---

#### _GstApp.AppSrc.set\_max\_bytes_ 


`def GstApp.AppSrc.set_max_bytes (self, max):
    #python wrapper for 'gst_app_src_set_max_bytes'`

Set the maximum amount of bytes that can be queued in _appsrc_. After the maximum amount of bytes are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the maximum number of bytes to queue

---

#### _gst\_app\_src\_set\_max\_time_ 


gst_app_src_set_max_time ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                          [GstClockTime](gstreamer/gstclock.html#GstClockTime "GstClockTime") max)

Set the maximum amount of time that can be queued in _appsrc_. After the maximum amount of time are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_–

the maximum amonut of time to queue

**Since** : 1.20

---

#### _GstApp.AppSrc.prototype.set\_max\_time_ 


`function GstApp.AppSrc.prototype.set_max_time(max: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_max_time'
}`

Set the maximum amount of time that can be queued in _appsrc_. After the maximum amount of time are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the maximum amonut of time to queue

**Since** : 1.20

---

#### _GstApp.AppSrc.set\_max\_time_ 


`def GstApp.AppSrc.set_max_time (self, max):
    #python wrapper for 'gst_app_src_set_max_time'`

Set the maximum amount of time that can be queued in _appsrc_. After the maximum amount of time are queued, _appsrc_ will emit the "enough-data" signal.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`max`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the maximum amonut of time to queue

**Since** : 1.20

---

#### _gst\_app\_src\_set\_simple\_callbacks_ 


gst_app_src_set_simple_callbacks ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                                  [GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb)

Set callbacks which will be executed when data is needed, enough data has been collected or when a seek should be performed. This is an alternative to using the signals, it has lower overhead and is thus less expensive, but also less flexible.

If callbacks are installed, no signals will be emitted for performance reasons.

Once _cb_ is set on an [GstAppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

Note that [gst\_app\_src\_set\_callbacks](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fcallbacks) and[gst\_app\_src\_set\_simple\_callbacks](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsimple%5Fcallbacks) are mutually exclusive and setting one will unset the other.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`cb`_ ( \[transfer: full\]\[nullable\])–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrc.prototype.set\_simple\_callbacks_ 


`function GstApp.AppSrc.prototype.set_simple_callbacks(cb: [GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks")): {
    // javascript wrapper for 'gst_app_src_set_simple_callbacks'
}`

Set callbacks which will be executed when data is needed, enough data has been collected or when a seek should be performed. This is an alternative to using the signals, it has lower overhead and is thus less expensive, but also less flexible.

If callbacks are installed, no signals will be emitted for performance reasons.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

Note that [gst\_app\_src\_set\_callbacks (not introspectable)](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fcallbacks) and[GstApp.AppSrc.prototype.set\_simple\_callbacks](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsimple%5Fcallbacks) are mutually exclusive and setting one will unset the other.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrc.set\_simple\_callbacks_ 


`def GstApp.AppSrc.set_simple_callbacks (self, cb):
    #python wrapper for 'gst_app_src_set_simple_callbacks'`

Set callbacks which will be executed when data is needed, enough data has been collected or when a seek should be performed. This is an alternative to using the signals, it has lower overhead and is thus less expensive, but also less flexible.

If callbacks are installed, no signals will be emitted for performance reasons.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

Note that [gst\_app\_src\_set\_callbacks (not introspectable)](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fcallbacks) and[GstApp.AppSrc.set\_simple\_callbacks](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fsimple%5Fcallbacks) are mutually exclusive and setting one will unset the other.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

#### _gst\_app\_src\_set\_size_ 


gst_app_src_set_size ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                      [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64") size)

Set the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`size`_–

the size to set

---

#### _GstApp.AppSrc.prototype.set\_size_ 


`function GstApp.AppSrc.prototype.set_size(size: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript wrapper for 'gst_app_src_set_size'
}`

Set the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`size`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the size to set

---

#### _GstApp.AppSrc.set\_size_ 


`def GstApp.AppSrc.set_size (self, size):
    #python wrapper for 'gst_app_src_set_size'`

Set the size of the stream in bytes. A value of -1 means that the size is not known.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`size`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the size to set

---

#### _gst\_app\_src\_set\_stream\_type_ 


gst_app_src_set_stream_type ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                             [GstAppStreamType](applib/gstappsrc.html#GstAppStreamType "GstAppStreamType") type)

Set the stream type on _appsrc_. For seekable streams, the "seek" signal must be connected to.

A stream\_type stream

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`type`_–

the new state

---

#### _GstApp.AppSrc.prototype.set\_stream\_type_ 


`function GstApp.AppSrc.prototype.set_stream_type(type: [GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType")): {
    // javascript wrapper for 'gst_app_src_set_stream_type'
}`

Set the stream type on _appsrc_. For seekable streams, the "seek" signal must be connected to.

A stream\_type stream

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`type`_ ([GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType"))–

the new state

---

#### _GstApp.AppSrc.set\_stream\_type_ 


`def GstApp.AppSrc.set_stream_type (self, type):
    #python wrapper for 'gst_app_src_set_stream_type'`

Set the stream type on _appsrc_. For seekable streams, the "seek" signal must be connected to.

A stream\_type stream

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`type`_ ([GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType"))–

the new state

---

### Signals

#### _enough-data_ 


enough_data_callback ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * self,
                      [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Signal that the source has enough data. It is recommended that the application stops calling push-buffer until the need-data signal is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`self`_–

_No description available_ 

__`userdata`_–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _enough-data_ 


`function enough_data_callback(self: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript callback for the 'enough-data' signal
}`

Signal that the source has enough data. It is recommended that the application stops calling push-buffer until the need-data signal is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _enough-data_ 


`def enough_data_callback (self, *user_data):
    #python callback for the 'enough-data' signal`

Signal that the source has enough data. It is recommended that the application stops calling push-buffer until the need-data signal is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _need-data_ 


need_data_callback ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * self,
                    [guint](https://docs.gtk.org/glib/types.html#guint "guint") length,
                    [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Signal that the source needs more data. In the callback or from another thread you should call push-buffer or end-of-stream.

_length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

You can call push-buffer multiple times until the enough-data signal is fired.

**Parameters:**

__`self`_–

_No description available_ 

__`length`_–

the amount of bytes needed.

__`userdata`_–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _need-data_ 


`function need_data_callback(self: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), length: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript callback for the 'need-data' signal
}`

Signal that the source needs more data. In the callback or from another thread you should call push-buffer or end-of-stream.

_length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

You can call push-buffer multiple times until the enough-data signal is fired.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`length`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the amount of bytes needed.

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _need-data_ 


`def need_data_callback (self, length, *user_data):
    #python callback for the 'need-data' signal`

Signal that the source needs more data. In the callback or from another thread you should call push-buffer or end-of-stream.

_length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

You can call push-buffer multiple times until the enough-data signal is fired.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`length`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the amount of bytes needed.

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _seek-data_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
seek_data_callback ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * self,
                    [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset,
                    [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Seek to the given offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`self`_–

_No description available_ 

__`offset`_–

the offset to seek to

__`userdata`_–

_No description available_ 

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the seek succeeded.

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _seek-data_ 


`function seek_data_callback(self: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript callback for the 'seek-data' signal
}`

Seek to the given offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

the offset to seek to

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the seek succeeded.

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

#### _seek-data_ 


`def seek_data_callback (self, offset, *user_data):
    #python callback for the 'seek-data' signal`

Seek to the given offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

the offset to seek to

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if the seek succeeded.

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) 

---

### Action Signals

#### _end-of-stream_ 


g_signal_emit_by_name (self, "end-of-stream", user_data, &ret);

Notify _appsrc_ that no more buffer are available.

**Parameters:**

__`self`_ ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") \*)–

_No description available_ 

__`userdata`_ ([gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer"))–

_No description available_ 

**Returns**–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _end-of-stream_ 


let ret = self.emit ("end-of-stream", user_data);

Notify _appsrc_ that no more buffer are available.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _end-of-stream_ 


ret = self.emit ("end-of-stream", user_data)

Notify _appsrc_ that no more buffer are available.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _push-buffer_ 


g_signal_emit_by_name (self, "push-buffer", buffer, user_data, &ret);

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad.

This function does not take ownership of the buffer, but it takes a reference so the buffer can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") \*)–

_No description available_ 

__`buffer`_ ([GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") \*, \[transfer: none\])–

a buffer to push

__`userdata`_ ([gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer"))–

_No description available_ 

**Returns**–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _push-buffer_ 


let ret = self.emit ("push-buffer", buffer, user_data);

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad.

This function does not take ownership of the buffer, but it takes a reference so the buffer can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

a buffer to push

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _push-buffer_ 


ret = self.emit ("push-buffer", buffer, user_data)

Adds a buffer to the queue of buffers that the appsrc element will push to its source pad.

This function does not take ownership of the buffer, but it takes a reference so the buffer can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

a buffer to push

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

---

#### _push-buffer-list_ 


g_signal_emit_by_name (self, "push-buffer-list", buffer_list, user_data, &ret);

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad.

This function does not take ownership of the buffer list, but it takes a reference so the buffer list can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") \*)–

_No description available_ 

__`bufferlist`_ ([GstBufferList](gstreamer/gstbufferlist.html#GstBufferList "GstBufferList") \*, \[transfer: none\])–

a buffer list to push

__`userdata`_ ([gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer"))–

_No description available_ 

**Returns**–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.14

---

#### _push-buffer-list_ 


let ret = self.emit ("push-buffer-list", buffer_list, user_data);

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad.

This function does not take ownership of the buffer list, but it takes a reference so the buffer list can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a buffer list to push

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.14

---

#### _push-buffer-list_ 


ret = self.emit ("push-buffer-list", buffer_list, user_data)

Adds a buffer list to the queue of buffers and buffer lists that the appsrc element will push to its source pad.

This function does not take ownership of the buffer list, but it takes a reference so the buffer list can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

a buffer list to push

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.14

---

#### _push-sample_ 


g_signal_emit_by_name (self, "push-sample", sample, user_data, &ret);

Extract a buffer from the provided sample and adds the extracted buffer to the queue of buffers that the appsrc element will push to its source pad. This function set the appsrc caps based on the caps in the sample and reset the caps if they change. Only the caps and the buffer of the provided sample are used and not for example the segment in the sample.

This function does not take ownership of the sample, but it takes a reference so the sample can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") \*)–

_No description available_ 

__`sample`_ ([GstSample](gstreamer/gstsample.html#GstSample "GstSample") \*, \[transfer: none\])–

a sample from which extract buffer to push

__`userdata`_ ([gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer"))–

_No description available_ 

**Returns**–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.6

---

#### _push-sample_ 


let ret = self.emit ("push-sample", sample, user_data);

Extract a buffer from the provided sample and adds the extracted buffer to the queue of buffers that the appsrc element will push to its source pad. This function set the appsrc caps based on the caps in the sample and reset the caps if they change. Only the caps and the buffer of the provided sample are used and not for example the segment in the sample.

This function does not take ownership of the sample, but it takes a reference so the sample can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

a sample from which extract buffer to push

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.6

---

#### _push-sample_ 


ret = self.emit ("push-sample", sample, user_data)

Extract a buffer from the provided sample and adds the extracted buffer to the queue of buffers that the appsrc element will push to its source pad. This function set the appsrc caps based on the caps in the sample and reset the caps if they change. Only the caps and the buffer of the provided sample are used and not for example the segment in the sample.

This function does not take ownership of the sample, but it takes a reference so the sample can be unreffed at any time after calling this function.

When the block property is TRUE, this function can block until free space becomes available in the queue.

**Parameters:**

__`self`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

a sample from which extract buffer to push

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

**Flags:** [Run Last](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-RUN-LAST:CAPS) / [Action](https://developer.gnome.org/gobject/unstable/gobject-Signals.html#G-SIGNAL-ACTION:CAPS) 

**Since** : 1.6

---

### Properties

#### _block_ 


“block” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

When max-bytes are queued and after the enough-data signal has been emitted, block any further push-buffer calls until the amount of queued bytes drops below the max-bytes limit.

 Flags : Read / Write

---

#### _block_ 


“block” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

When max-bytes are queued and after the enough-data signal has been emitted, block any further push-buffer calls until the amount of queued bytes drops below the max-bytes limit.

 Flags : Read / Write

---

#### _block_ 


“self.props.block” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

When max-bytes are queued and after the enough-data signal has been emitted, block any further push-buffer calls until the amount of queued bytes drops below the max-bytes limit.

 Flags : Read / Write

---

#### _caps_ 


“caps” [GstCaps](gstreamer/gstcaps.html#GstCaps "GstCaps") *

The GstCaps that will negotiated downstream and will be put on outgoing buffers.

 Flags : Read / Write

---

#### _caps_ 


“caps” [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")

The GstCaps that will negotiated downstream and will be put on outgoing buffers.

 Flags : Read / Write

---

#### _caps_ 


“self.props.caps” [Gst.Caps](gstreamer/gstcaps.html#GstCaps "Gst.Caps")

The GstCaps that will negotiated downstream and will be put on outgoing buffers.

 Flags : Read / Write

---

#### _current-level-buffers_ 


“current-level-buffers” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The number of currently queued buffers inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _current-level-buffers_ 


“current-level-buffers” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The number of currently queued buffers inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _current\_level\_buffers_ 


“self.props.current_level_buffers” [int](https://docs.python.org/3/library/functions.html#int "int")

The number of currently queued buffers inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _current-level-bytes_ 


“current-level-bytes” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The number of currently queued bytes inside appsrc.

 Flags : Read

**Since** : 1.2

---

#### _current-level-bytes_ 


“current-level-bytes” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The number of currently queued bytes inside appsrc.

 Flags : Read

**Since** : 1.2

---

#### _current\_level\_bytes_ 


“self.props.current_level_bytes” [int](https://docs.python.org/3/library/functions.html#int "int")

The number of currently queued bytes inside appsrc.

 Flags : Read

**Since** : 1.2

---

#### _current-level-time_ 


“current-level-time” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The amount of currently queued time inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _current-level-time_ 


“current-level-time” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The amount of currently queued time inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _current\_level\_time_ 


“self.props.current_level_time” [int](https://docs.python.org/3/library/functions.html#int "int")

The amount of currently queued time inside appsrc.

 Flags : Read

**Since** : 1.20

---

#### _dropped_ 


“dropped” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

Number of buffers that were dropped.

 Flags : Read

**Since** : 1.28

---

#### _dropped_ 


“dropped” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Number of buffers that were dropped.

 Flags : Read

**Since** : 1.28

---

#### _dropped_ 


“self.props.dropped” [int](https://docs.python.org/3/library/functions.html#int "int")

Number of buffers that were dropped.

 Flags : Read

**Since** : 1.28

---

#### _duration_ 


“duration” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The total duration in nanoseconds of the data stream. If the total duration is known, it is recommended to configure it with this property.

 Flags : Read / Write

**Since** : 1.10

---

#### _duration_ 


“duration” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The total duration in nanoseconds of the data stream. If the total duration is known, it is recommended to configure it with this property.

 Flags : Read / Write

**Since** : 1.10

---

#### _duration_ 


“self.props.duration” [int](https://docs.python.org/3/library/functions.html#int "int")

The total duration in nanoseconds of the data stream. If the total duration is known, it is recommended to configure it with this property.

 Flags : Read / Write

**Since** : 1.10

---

#### _emit-signals_ 


“emit-signals” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

Make appsrc emit the "need-data", "enough-data" and "seek-data" signals. This option is by default enabled for backwards compatibility reasons but can disabled when needed because signal emission is expensive.

 Flags : Read / Write

---

#### _emit-signals_ 


“emit-signals” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Make appsrc emit the "need-data", "enough-data" and "seek-data" signals. This option is by default enabled for backwards compatibility reasons but can disabled when needed because signal emission is expensive.

 Flags : Read / Write

---

#### _emit\_signals_ 


“self.props.emit_signals” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

Make appsrc emit the "need-data", "enough-data" and "seek-data" signals. This option is by default enabled for backwards compatibility reasons but can disabled when needed because signal emission is expensive.

 Flags : Read / Write

---

#### _format_ 


“format” [GstFormat](gstreamer/gstformat.html#GstFormat "GstFormat") *

The format to use for segment events. When the source is producing timestamped buffers this property should be set to GST\_FORMAT\_TIME.

 Flags : Read / Write

---

#### _format_ 


“format” [Gst.Format](gstreamer/gstformat.html#GstFormat "Gst.Format")

The format to use for segment events. When the source is producing timestamped buffers this property should be set to GST\_FORMAT\_TIME.

 Flags : Read / Write

---

#### _format_ 


“self.props.format” [Gst.Format](gstreamer/gstformat.html#GstFormat "Gst.Format")

The format to use for segment events. When the source is producing timestamped buffers this property should be set to GST\_FORMAT\_TIME.

 Flags : Read / Write

---

#### _handle-segment-change_ 


“handle-segment-change” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

When enabled, appsrc will check GstSegment in GstSample which was pushed via [gst\_app\_src\_push\_sample](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fpush%5Fsample) or "push-sample" signal action. If a GstSegment is changed, corresponding segment event will be followed by next data flow.

FIXME: currently only GST\_FORMAT\_TIME format is supported and therefore GstAppSrc::format should be time. However, possibly [GstAppSrc](applib/gstappsrc.html#GstAppSrc) can support other formats.

 Flags : Read / Write

**Since** : 1.18

---

#### _handle-segment-change_ 


“handle-segment-change” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

When enabled, appsrc will check GstSegment in GstSample which was pushed via [GstApp.AppSrc.prototype.push\_sample](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fpush%5Fsample) or "push-sample" signal action. If a GstSegment is changed, corresponding segment event will be followed by next data flow.

FIXME: currently only GST\_FORMAT\_TIME format is supported and therefore GstAppSrc::format should be time. However, possibly [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) can support other formats.

 Flags : Read / Write

**Since** : 1.18

---

#### _handle\_segment\_change_ 


“self.props.handle_segment_change” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

When enabled, appsrc will check GstSegment in GstSample which was pushed via [GstApp.AppSrc.push\_sample](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fpush%5Fsample) or "push-sample" signal action. If a GstSegment is changed, corresponding segment event will be followed by next data flow.

FIXME: currently only GST\_FORMAT\_TIME format is supported and therefore GstAppSrc::format should be time. However, possibly [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) can support other formats.

 Flags : Read / Write

**Since** : 1.18

---

#### _in_ 


“in” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

Number of input buffers that were queued.

 Flags : Read

**Since** : 1.28

---

#### _in_ 


“in” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Number of input buffers that were queued.

 Flags : Read

**Since** : 1.28

---

#### _in_ 


“self.props.in” [int](https://docs.python.org/3/library/functions.html#int "int")

Number of input buffers that were queued.

 Flags : Read

**Since** : 1.28

---

#### _is-live_ 


“is-live” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

Instruct the source to behave like a live source. This includes that it will only push out buffers in the PLAYING state.

 Flags : Read / Write

---

#### _is-live_ 


“is-live” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Instruct the source to behave like a live source. This includes that it will only push out buffers in the PLAYING state.

 Flags : Read / Write

---

#### _is\_live_ 


“self.props.is_live” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

Instruct the source to behave like a live source. This includes that it will only push out buffers in the PLAYING state.

 Flags : Read / Write

---

#### _leaky-type_ 


“leaky-type” [GstAppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstAppLeakyType") *

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

 Flags : Read / Write

**Since** : 1.20

---

#### _leaky-type_ 


“leaky-type” [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType")

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

 Flags : Read / Write

**Since** : 1.20

---

#### _leaky\_type_ 


“self.props.leaky_type” [GstApp.AppLeakyType](applib/gstappsrc.html#GstAppLeakyType "GstApp.AppLeakyType")

When set to any other value than GST\_APP\_LEAKY\_TYPE\_NONE then the appsrc will drop any buffers that are pushed into it once its internal queue is full. The selected type defines whether to drop the oldest or new buffers.

 Flags : Read / Write

**Since** : 1.20

---

#### _max-buffers_ 


“max-buffers” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The maximum amount of buffers that can be queued internally. After the maximum amount of buffers are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _max-buffers_ 


“max-buffers” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The maximum amount of buffers that can be queued internally. After the maximum amount of buffers are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _max\_buffers_ 


“self.props.max_buffers” [int](https://docs.python.org/3/library/functions.html#int "int")

The maximum amount of buffers that can be queued internally. After the maximum amount of buffers are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _max-bytes_ 


“max-bytes” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The maximum amount of bytes that can be queued internally. After the maximum amount of bytes are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

---

#### _max-bytes_ 


“max-bytes” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The maximum amount of bytes that can be queued internally. After the maximum amount of bytes are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

---

#### _max\_bytes_ 


“self.props.max_bytes” [int](https://docs.python.org/3/library/functions.html#int "int")

The maximum amount of bytes that can be queued internally. After the maximum amount of bytes are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

---

#### _max-latency_ 


“max-latency” [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64")

 Flags : Read / Write

---

#### _max-latency_ 


“max-latency” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

 Flags : Read / Write

---

#### _max\_latency_ 


“self.props.max_latency” [int](https://docs.python.org/3/library/functions.html#int "int")

 Flags : Read / Write

---

#### _max-time_ 


“max-time” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

The maximum amount of time that can be queued internally. After the maximum amount of time are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _max-time_ 


“max-time” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The maximum amount of time that can be queued internally. After the maximum amount of time are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _max\_time_ 


“self.props.max_time” [int](https://docs.python.org/3/library/functions.html#int "int")

The maximum amount of time that can be queued internally. After the maximum amount of time are queued, appsrc will emit the "enough-data" signal.

 Flags : Read / Write

**Since** : 1.20

---

#### _min-latency_ 


“min-latency” [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64")

The minimum latency of the source. A value of -1 will use the default latency calculations of [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc).

 Flags : Read / Write

---

#### _min-latency_ 


“min-latency” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The minimum latency of the source. A value of -1 will use the default latency calculations of [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc).

 Flags : Read / Write

---

#### _min\_latency_ 


“self.props.min_latency” [int](https://docs.python.org/3/library/functions.html#int "int")

The minimum latency of the source. A value of -1 will use the default latency calculations of [GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc).

 Flags : Read / Write

---

#### _min-percent_ 


“min-percent” [guint](https://docs.gtk.org/glib/types.html#guint "guint")

Make appsrc emit the "need-data" signal when the amount of bytes in the queue drops below this percentage of max-bytes.

 Flags : Read / Write

---

#### _min-percent_ 


“min-percent” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Make appsrc emit the "need-data" signal when the amount of bytes in the queue drops below this percentage of max-bytes.

 Flags : Read / Write

---

#### _min\_percent_ 


“self.props.min_percent” [int](https://docs.python.org/3/library/functions.html#int "int")

Make appsrc emit the "need-data" signal when the amount of bytes in the queue drops below this percentage of max-bytes.

 Flags : Read / Write

---

#### _out_ 


“out” [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64")

Number of output buffers that were dequeued.

 Flags : Read

**Since** : 1.28

---

#### _out_ 


“out” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Number of output buffers that were dequeued.

 Flags : Read

**Since** : 1.28

---

#### _out_ 


“self.props.out” [int](https://docs.python.org/3/library/functions.html#int "int")

Number of output buffers that were dequeued.

 Flags : Read

**Since** : 1.28

---

#### _silent_ 


“silent” [gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")

Don't emit notify for input, output and dropped buffers.

 Flags : Read / Write

**Since** : 1.28

---

#### _silent_ 


“silent” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

Don't emit notify for input, output and dropped buffers.

 Flags : Read / Write

**Since** : 1.28

---

#### _silent_ 


“self.props.silent” [bool](https://docs.python.org/3/library/functions.html#bool "bool")

Don't emit notify for input, output and dropped buffers.

 Flags : Read / Write

**Since** : 1.28

---

#### _size_ 


“size” [gint64](https://docs.gtk.org/glib/types.html#gint64 "gint64")

The total size in bytes of the data stream. If the total size is known, it is recommended to configure it with this property.

 Flags : Read / Write

---

#### _size_ 


“size” [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")

The total size in bytes of the data stream. If the total size is known, it is recommended to configure it with this property.

 Flags : Read / Write

---

#### _size_ 


“self.props.size” [int](https://docs.python.org/3/library/functions.html#int "int")

The total size in bytes of the data stream. If the total size is known, it is recommended to configure it with this property.

 Flags : Read / Write

---

#### _stream-type_ 


“stream-type” [GstAppStreamType](applib/gstappsrc.html#GstAppStreamType "GstAppStreamType") *

The type of stream that this source is producing. For seekable streams the application should connect to the seek-data signal.

 Flags : Read / Write

---

#### _stream-type_ 


“stream-type” [GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType")

The type of stream that this source is producing. For seekable streams the application should connect to the seek-data signal.

 Flags : Read / Write

---

#### _stream\_type_ 


“self.props.stream_type” [GstApp.AppStreamType](applib/gstappsrc.html#GstAppStreamType "GstApp.AppStreamType")

The type of stream that this source is producing. For seekable streams the application should connect to the seek-data signal.

 Flags : Read / Write

---

### Virtual Methods

#### _end\_of\_stream_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
end_of_stream ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

**Parameters:**

__`appsrc`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_end\_of\_stream_ 


`function vfunc_end_of_stream(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc")): {
    // javascript implementation of the 'end_of_stream' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_end\_of\_stream_ 


`def do_end_of_stream (appsrc):
    #python implementation of the 'end_of_stream' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _enough\_data_ 


enough_data ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc)

**Parameters:**

__`appsrc`_–

_No description available_ 

---

#### _vfunc\_enough\_data_ 


`function vfunc_enough_data(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc")): {
    // javascript implementation of the 'enough_data' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

---

#### _do\_enough\_data_ 


`def do_enough_data (appsrc):
    #python implementation of the 'enough_data' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

---

#### _need\_data_ 


need_data ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
           [guint](https://docs.gtk.org/glib/types.html#guint "guint") length)

**Parameters:**

__`appsrc`_–

_No description available_ 

__`length`_–

_No description available_ 

---

#### _vfunc\_need\_data_ 


`function vfunc_need_data(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), length: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript implementation of the 'need_data' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`length`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_need\_data_ 


`def do_need_data (appsrc, length):
    #python implementation of the 'need_data' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`length`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

---

#### _push\_buffer_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
push_buffer ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
             [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") * buffer)

**Parameters:**

__`appsrc`_–

_No description available_ 

__`buffer`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_push\_buffer_ 


`function vfunc_push_buffer(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), buffer: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'push_buffer' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_push\_buffer_ 


`def do_push_buffer (appsrc, buffer):
    #python implementation of the 'push_buffer' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`buffer`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _push\_buffer\_list_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
push_buffer_list ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                  [GstBufferList](gstreamer/gstbufferlist.html#GstBufferList "GstBufferList") * buffer_list)

**Parameters:**

__`appsrc`_–

_No description available_ 

__`bufferlist`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_push\_buffer\_list_ 


`function vfunc_push_buffer_list(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), buffer_list: [Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList")): {
    // javascript implementation of the 'push_buffer_list' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_push\_buffer\_list_ 


`def do_push_buffer_list (appsrc, buffer_list):
    #python implementation of the 'push_buffer_list' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`bufferlist`_ ([Gst.BufferList](gstreamer/gstbufferlist.html#GstBufferList "Gst.BufferList"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _push\_sample_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
push_sample ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
             [GstSample](gstreamer/gstsample.html#GstSample "GstSample") * sample)

**Parameters:**

__`appsrc`_–

_No description available_ 

__`sample`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_push\_sample_ 


`function vfunc_push_sample(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), sample: [Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample")): {
    // javascript implementation of the 'push_sample' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_push\_sample_ 


`def do_push_sample (appsrc, sample):
    #python implementation of the 'push_sample' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`sample`_ ([Gst.Sample](gstreamer/gstsample.html#GstSample "Gst.Sample"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _seek\_data_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
seek_data ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
           [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset)

**Parameters:**

__`appsrc`_–

_No description available_ 

__`offset`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_seek\_data_ 


`function vfunc_seek_data(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number")): {
    // javascript implementation of the 'seek_data' virtual method
}`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

_No description available_ 

---

#### _do\_seek\_data_ 


`def do_seek_data (appsrc, offset):
    #python implementation of the 'seek_data' virtual method`

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

_No description available_ 

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

_No description available_ 

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

_No description available_ 

---

## _GstAppSrcCallbacks_ 

A set of callbacks that can be installed on the appsrc with[gst\_app\_src\_set\_callbacks](applib/gstappsrc.html#gst%5Fapp%5Fsrc%5Fset%5Fcallbacks).

---

## _GstAppSrcSimpleCallbacks_ 

A set of callbacks that can be installed on the appsink with[gst\_app\_sink\_set\_simple\_callbacks](applib/gstappsink.html#gst%5Fapp%5Fsink%5Fset%5Fsimple%5Fcallbacks).

Unlike GstAppSrcCallbacks this can also be used from bindings.

**Since** : 1.28

---

## _GstApp.AppSrcSimpleCallbacks_ 

A set of callbacks that can be installed on the appsink with[GstApp.AppSink.prototype.set\_simple\_callbacks](applib/gstappsink.html#gst%5Fapp%5Fsink%5Fset%5Fsimple%5Fcallbacks).

Unlike GstAppSrcCallbacks this can also be used from bindings.

**Since** : 1.28

---

## _GstApp.AppSrcSimpleCallbacks_ 

A set of callbacks that can be installed on the appsink with[GstApp.AppSink.set\_simple\_callbacks](applib/gstappsink.html#gst%5Fapp%5Fsink%5Fset%5Fsimple%5Fcallbacks).

Unlike GstAppSrcCallbacks this can also be used from bindings.

**Since** : 1.28

---

### Constructors

#### _gst\_app\_src\_simple\_callbacks\_new_ 


[GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") *
gst_app_src_simple_callbacks_new ()

Creates a new instance of callbacks.

**Returns** ( \[transfer: full\])–

New empty GstAppSrcSimpleCallbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.new_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.new(): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_new'
}`

Creates a new instance of callbacks.

**Returns** ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

New empty GstAppSrcSimpleCallbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.new_ 


`def GstApp.AppSrcSimpleCallbacks.new ():
    #python wrapper for 'gst_app_src_simple_callbacks_new'`

Creates a new instance of callbacks.

**Returns** ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

New empty GstAppSrcSimpleCallbacks

**Since** : 1.28

---

### Methods

#### _gst\_app\_src\_simple\_callbacks\_ref_ 


[GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") *
gst_app_src_simple_callbacks_ref ([GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb)

Increases the reference count of _cb_.

**Parameters:**

__`cb`_–

the callbacks

**Returns** ( \[transfer: full\])–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.ref_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.ref(): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_ref'
}`

Increases the reference count of _cb_.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Returns** ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.ref_ 


`def GstApp.AppSrcSimpleCallbacks.ref (self):
    #python wrapper for 'gst_app_src_simple_callbacks_ref'`

Increases the reference count of _cb_.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Returns** ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

#### _gst\_app\_src\_simple\_callbacks\_set\_enough\_data_ 


gst_app_src_simple_callbacks_set_enough_data ([GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb,
                                              [GstAppSrcEnoughDataCallback](applib/gstappsrc.html#GstAppSrcEnoughDataCallback "GstAppSrcEnoughDataCallback") enough_data_cb,
                                              [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data,
                                              [GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html "GDestroyNotify") destroy_notify)

Sets the enough data callback on _cb_.

Once _cb_ is set on an [GstAppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_–

the callbacks

__`enoughdatacb`_ ( \[scope notified\]\[closure\])–

EOS callback

__`userdata`_–

the user data

__`destroynotify`_–

[GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html) to free the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.set\_enough\_data_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.set_enough_data(enough_data_cb: [GstApp.AppSrcEnoughDataCallback](applib/gstappsrc.html#GstAppSrcEnoughDataCallback "GstApp.AppSrcEnoughDataCallback"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_set_enough_data'
}`

Sets the enough data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`enoughdatacb`_ ([GstApp.AppSrcEnoughDataCallback](applib/gstappsrc.html#GstAppSrcEnoughDataCallback "GstApp.AppSrcEnoughDataCallback"))–

EOS callback

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.set\_enough\_data_ 


`def GstApp.AppSrcSimpleCallbacks.set_enough_data (self, enough_data_cb, *user_data):
    #python wrapper for 'gst_app_src_simple_callbacks_set_enough_data'`

Sets the enough data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`enoughdatacb`_ ([GstApp.AppSrcEnoughDataCallback](applib/gstappsrc.html#GstAppSrcEnoughDataCallback "GstApp.AppSrcEnoughDataCallback"))–

EOS callback

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

the user data

**Since** : 1.28

---

#### _gst\_app\_src\_simple\_callbacks\_set\_need\_data_ 


gst_app_src_simple_callbacks_set_need_data ([GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb,
                                            [GstAppSrcNeedDataCallback](applib/gstappsrc.html#GstAppSrcNeedDataCallback "GstAppSrcNeedDataCallback") need_data_cb,
                                            [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data,
                                            [GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html "GDestroyNotify") destroy_notify)

Sets the need data callback on _cb_.

Once _cb_ is set on an [GstAppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_–

the callbacks

__`needdatacb`_ ( \[scope notified\]\[closure\])–

EOS callback

__`userdata`_–

the user data

__`destroynotify`_–

[GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html) to free the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.set\_need\_data_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.set_need_data(need_data_cb: [GstApp.AppSrcNeedDataCallback](applib/gstappsrc.html#GstAppSrcNeedDataCallback "GstApp.AppSrcNeedDataCallback"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_set_need_data'
}`

Sets the need data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`needdatacb`_ ([GstApp.AppSrcNeedDataCallback](applib/gstappsrc.html#GstAppSrcNeedDataCallback "GstApp.AppSrcNeedDataCallback"))–

EOS callback

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.set\_need\_data_ 


`def GstApp.AppSrcSimpleCallbacks.set_need_data (self, need_data_cb, *user_data):
    #python wrapper for 'gst_app_src_simple_callbacks_set_need_data'`

Sets the need data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`needdatacb`_ ([GstApp.AppSrcNeedDataCallback](applib/gstappsrc.html#GstAppSrcNeedDataCallback "GstApp.AppSrcNeedDataCallback"))–

EOS callback

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

the user data

**Since** : 1.28

---

#### _gst\_app\_src\_simple\_callbacks\_set\_seek\_data_ 


gst_app_src_simple_callbacks_set_seek_data ([GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb,
                                            [GstAppSrcSeekDataCallback](applib/gstappsrc.html#GstAppSrcSeekDataCallback "GstAppSrcSeekDataCallback") seek_data_cb,
                                            [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data,
                                            [GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html "GDestroyNotify") destroy_notify)

Sets the seek data callback on _cb_.

Once _cb_ is set on an [GstAppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_–

the callbacks

__`seekdatacb`_ ( \[scope notified\]\[closure\])–

EOS callback

__`userdata`_–

the user data

__`destroynotify`_–

[GDestroyNotify](https://docs.gtk.org/glib/callback.DestroyNotify.html) to free the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.set\_seek\_data_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.set_seek_data(seek_data_cb: [GstApp.AppSrcSeekDataCallback](applib/gstappsrc.html#GstAppSrcSeekDataCallback "GstApp.AppSrcSeekDataCallback"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_set_seek_data'
}`

Sets the seek data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`seekdatacb`_ ([GstApp.AppSrcSeekDataCallback](applib/gstappsrc.html#GstAppSrcSeekDataCallback "GstApp.AppSrcSeekDataCallback"))–

EOS callback

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

the user data

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.set\_seek\_data_ 


`def GstApp.AppSrcSimpleCallbacks.set_seek_data (self, seek_data_cb, *user_data):
    #python wrapper for 'gst_app_src_simple_callbacks_set_seek_data'`

Sets the seek data callback on _cb_.

Once _cb_ is set on an [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc) it is not possible anymore to change any of the callbacks inside it.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

__`seekdatacb`_ ([GstApp.AppSrcSeekDataCallback](applib/gstappsrc.html#GstAppSrcSeekDataCallback "GstApp.AppSrcSeekDataCallback"))–

EOS callback

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

the user data

**Since** : 1.28

---

#### _gst\_app\_src\_simple\_callbacks\_unref_ 


gst_app_src_simple_callbacks_unref ([GstAppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstAppSrcSimpleCallbacks") * cb)

Decreases the reference count of _cb_ and frees it after the last reference is dropped.

**Parameters:**

__`cb`_–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.prototype.unref_ 


`function GstApp.AppSrcSimpleCallbacks.prototype.unref(): {
    // javascript wrapper for 'gst_app_src_simple_callbacks_unref'
}`

Decreases the reference count of _cb_ and frees it after the last reference is dropped.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

#### _GstApp.AppSrcSimpleCallbacks.unref_ 


`def GstApp.AppSrcSimpleCallbacks.unref (self):
    #python wrapper for 'gst_app_src_simple_callbacks_unref'`

Decreases the reference count of _cb_ and frees it after the last reference is dropped.

**Parameters:**

__`cb`_ ([GstApp.AppSrcSimpleCallbacks](applib/gstappsrc.html#GstAppSrcSimpleCallbacks "GstApp.AppSrcSimpleCallbacks"))–

the callbacks

**Since** : 1.28

---

## Function Macros

### _GST\_APP\_SRC\_CAST_ 


#define GST_APP_SRC_CAST(obj) \
  ((GstAppSrc*)(obj))

---

## Enumerations

### _GstAppLeakyType_ 

Buffer dropping scheme to avoid the element's internal queue to block when full.

##### Members

__`GSTAPPLEAKYTYPENONE`_ (0) –

Not Leaky

__`GSTAPPLEAKYTYPEUPSTREAM`_ (1) –

Leaky on upstream (new buffers)

__`GSTAPPLEAKYTYPEDOWNSTREAM`_ (2) –

Leaky on downstream (old buffers)

**Since** : 1.20

---

### _GstApp.AppLeakyType_ 

Buffer dropping scheme to avoid the element's internal queue to block when full.

##### Members

__`GstApp.AppLeakyType.NONE`_ (0) –

Not Leaky

__`GstApp.AppLeakyType.UPSTREAM`_ (1) –

Leaky on upstream (new buffers)

__`GstApp.AppLeakyType.DOWNSTREAM`_ (2) –

Leaky on downstream (old buffers)

**Since** : 1.20

---

### _GstApp.AppLeakyType_ 

Buffer dropping scheme to avoid the element's internal queue to block when full.

##### Members

__`GstApp.AppLeakyType.NONE`_ (0) –

Not Leaky

__`GstApp.AppLeakyType.UPSTREAM`_ (1) –

Leaky on upstream (new buffers)

__`GstApp.AppLeakyType.DOWNSTREAM`_ (2) –

Leaky on downstream (old buffers)

**Since** : 1.20

---

### _GstAppStreamType_ 

The stream type.

##### Members

__`GSTAPPSTREAMTYPESTREAM`_ (0) –

No seeking is supported in the stream, such as a live stream.

__`GSTAPPSTREAMTYPESEEKABLE`_ (1) –

The stream is seekable but seeking might not be very fast, such as data from a webserver.

__`GSTAPPSTREAMTYPERANDOMACCESS`_ (2) –

The stream is seekable and seeking is fast, such as in a local file.

---

### _GstApp.AppStreamType_ 

The stream type.

##### Members

__`GstApp.AppStreamType.STREAM`_ (0) –

No seeking is supported in the stream, such as a live stream.

__`GstApp.AppStreamType.SEEKABLE`_ (1) –

The stream is seekable but seeking might not be very fast, such as data from a webserver.

__`GstApp.AppStreamType.RANDOMACCESS`_ (2) –

The stream is seekable and seeking is fast, such as in a local file.

---

### _GstApp.AppStreamType_ 

The stream type.

##### Members

__`GstApp.AppStreamType.STREAM`_ (0) –

No seeking is supported in the stream, such as a live stream.

__`GstApp.AppStreamType.SEEKABLE`_ (1) –

The stream is seekable but seeking might not be very fast, such as data from a webserver.

__`GstApp.AppStreamType.RANDOMACCESS`_ (2) –

The stream is seekable and seeking is fast, such as in a local file.

---

## Constants

### _GST\_TYPE\_APP\_SRC_ 


#define GST_TYPE_APP_SRC \
  (gst_app_src_get_type())

---

### _GST\_TYPE\_APP\_SRC\_SIMPLE\_CALLBACKS_ 


#define GST_TYPE_APP_SRC_SIMPLE_CALLBACKS (gst_app_src_simple_callbacks_get_type ())

**Since** : 1.28

---

## Callbacks

### _GstAppSrcEnoughDataCallback_ 


(*GstAppSrcEnoughDataCallback) ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                                [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Called when appsrc has enough data. It is recommended that the application stops calling push-buffer until the need\_data callback is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`userdata`_–

callback user data

**Since** : 1.28

---

### _GstApp.AppSrcEnoughDataCallback_ 


`function GstApp.AppSrcEnoughDataCallback(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'GstAppSrcEnoughDataCallback'
}`

Called when appsrc has enough data. It is recommended that the application stops calling push-buffer until the need\_data callback is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

callback user data

**Since** : 1.28

---

### _GstApp.AppSrcEnoughDataCallback_ 


`def GstApp.AppSrcEnoughDataCallback (appsrc, *user_data):
    #python wrapper for 'GstAppSrcEnoughDataCallback'`

Called when appsrc has enough data. It is recommended that the application stops calling push-buffer until the need\_data callback is emitted again to avoid excessive buffer queueing.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

callback user data

**Since** : 1.28

---

### _GstAppSrcNeedDataCallback_ 


(*GstAppSrcNeedDataCallback) ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                              [guint](https://docs.gtk.org/glib/types.html#guint "guint") length,
                              [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Called when the appsrc needs more data. A buffer or EOS should be pushed to appsrc from this thread or another thread. _length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`length`_–

Length hint

__`userdata`_–

callback user data

**Since** : 1.28

---

### _GstApp.AppSrcNeedDataCallback_ 


`function GstApp.AppSrcNeedDataCallback(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), length: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'GstAppSrcNeedDataCallback'
}`

Called when the appsrc needs more data. A buffer or EOS should be pushed to appsrc from this thread or another thread. _length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`length`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

Length hint

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

callback user data

**Since** : 1.28

---

### _GstApp.AppSrcNeedDataCallback_ 


`def GstApp.AppSrcNeedDataCallback (appsrc, length, *user_data):
    #python wrapper for 'GstAppSrcNeedDataCallback'`

Called when the appsrc needs more data. A buffer or EOS should be pushed to appsrc from this thread or another thread. _length_ is just a hint and when it is set to -1, any number of bytes can be pushed into _appsrc_.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`length`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

Length hint

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

callback user data

**Since** : 1.28

---

### _GstAppSrcSeekDataCallback_ 


[gboolean](https://docs.gtk.org/glib/types.html#gboolean "gboolean")
(*GstAppSrcSeekDataCallback) ([GstAppSrc](applib/gstappsrc.html#GstAppSrc "GstAppSrc") * appsrc,
                              [guint64](https://docs.gtk.org/glib/types.html#guint64 "guint64") offset,
                              [gpointer](https://docs.gtk.org/glib/types.html#gpointer "gpointer") user_data)

Called when a seek should be performed to the offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`appsrc`_–

a [GstAppSrc](applib/gstappsrc.html#GstAppSrc)

__`offset`_–

Offset to seek to.

__`userdata`_–

callback user data

**Returns**–

[TRUE](https://web.mit.edu/barnowl/share/gtk-doc/html/glib/glib-Standard-Macros.html#TRUE:CAPS) if the seek was successful.

**Since** : 1.28

---

### _GstApp.AppSrcSeekDataCallback_ 


`function GstApp.AppSrcSeekDataCallback(appsrc: [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"), offset: [Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"), user_data: [Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object")): {
    // javascript wrapper for 'GstAppSrcSeekDataCallback'
}`

Called when a seek should be performed to the offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`offset`_ ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

Offset to seek to.

__`userdata`_ ([Object](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Object "Object"))–

callback user data

**Returns** ([Number](https://developer.mozilla.org/en-US/docs/Glossary/Number "Number"))–

[true](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global%5FObjects/Boolean) if the seek was successful.

**Since** : 1.28

---

### _GstApp.AppSrcSeekDataCallback_ 


`def GstApp.AppSrcSeekDataCallback (appsrc, offset, *user_data):
    #python wrapper for 'GstAppSrcSeekDataCallback'`

Called when a seek should be performed to the offset. The next push-buffer should produce buffers from the new _offset_. This callback is only called for seekable stream types.

**Parameters:**

__`appsrc`_ ([GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc "GstApp.AppSrc"))–

a [GstApp.AppSrc](applib/gstappsrc.html#GstAppSrc)

__`offset`_ ([int](https://docs.python.org/3/library/functions.html#int "int"))–

Offset to seek to.

__`userdata`_ ([variadic](https://docs.python.org/dev/tutorial/controlflow.html#arbitrary-argument-lists "variadic"))–

callback user data

**Returns** ([bool](https://docs.python.org/3/library/functions.html#bool "bool"))–

[True](https://docs.python.org/3/library/constants.html#True) if the seek was successful.

**Since** : 1.28

---

The results of the search are