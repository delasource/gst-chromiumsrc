Title: GstPushSrc

URL Source: https://gstreamer.freedesktop.org/documentation/base/gstpushsrc.html

Markdown Content:
# GstPushSrc

This class is mostly useful for elements that cannot do random access, or at least very slowly. The source usually prefers to push out a fixed size buffer.

Subclasses usually operate in a format that is different from the default GST\_FORMAT\_BYTES format of [GstBaseSrc](base/gstbasesrc.html#GstBaseSrc).

Classes extending this base class will usually be scheduled in a push based mode. If the peer accepts to operate without offsets and within the limits of the allowed block size, this class can operate in getrange based mode automatically. To make this possible, the subclass should implement and override the SCHEDULING query.

The subclass should extend the methods from the baseclass in addition to the ::create method.

Seeking, flushing, scheduling and sync is all handled by this base class.

## _GstPushSrc_ 


[GObject](https://docs.gtk.org/gobject/class.Object.html "GObject")
    ╰──[GInitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GInitiallyUnowned")
        ╰──[GstObject](gstreamer/gstobject.html#GstObject "GstObject")
            ╰──[GstElement](gstreamer/gstelement.html#GstElement "GstElement")
                ╰──[GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc")
                    ╰──GstPushSrc

The opaque [GstPushSrc](base/gstpushsrc.html#GstPushSrc) data structure.

### Members

__`parent`_ ([GstBaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBaseSrc")) –

_No description available_ 

---

### Class structure

#### _GstPushSrcClass_ 

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _fill_ method should be overridden to produce buffers.

##### Fields

__`parentclass`_ (<GstBaseSrcClass>) –

Element parent class

---

###  GstBase.PushSrcClass

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _fill_ method should be overridden to produce buffers.

##### Attributes

__`parentclass`_ ([GstBase.BaseSrcClass](GstBaseSrcClass "GstBase.BaseSrcClass")) –

Element parent class

---

###  GstBase.PushSrcClass

Subclasses can override any of the available virtual methods or not, as needed. At the minimum, the _fill_ method should be overridden to produce buffers.

##### Attributes

__`parentclass`_ ([GstBase.BaseSrcClass](GstBaseSrcClass "GstBase.BaseSrcClass")) –

Element parent class

---

## _GstBase.PushSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")
                    ╰──GstBase.PushSrc

The opaque [GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc) data structure.

### Members

__`parent`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")) –

_No description available_ 

---

## _GstBase.PushSrc_ 


[GObject.Object](https://docs.gtk.org/gobject/class.Object.html "GObject.Object")
    ╰──[GObject.InitiallyUnowned](https://docs.gtk.org/gobject/class.InitiallyUnowned.html "GObject.InitiallyUnowned")
        ╰──[Gst.Object](gstreamer/gstobject.html#GstObject "Gst.Object")
            ╰──[Gst.Element](gstreamer/gstelement.html#GstElement "Gst.Element")
                ╰──[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")
                    ╰──GstBase.PushSrc

The opaque [GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc) data structure.

### Members

__`parent`_ ([GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc "GstBase.BaseSrc")) –

_No description available_ 

---

### Virtual Methods

#### _alloc_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
alloc ([GstPushSrc](base/gstpushsrc.html#GstPushSrc "GstPushSrc") * src,
       [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") ** buf)

Ask the subclass to allocate a buffer. The subclass decides which size this buffer should be. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_alloc_ 


`function vfunc_alloc(src: [GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc")): {
    // javascript implementation of the 'alloc' virtual method
}`

Ask the subclass to allocate a buffer. The subclass decides which size this buffer should be. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _do\_alloc_ 


`def do_alloc (src):
    #python implementation of the 'alloc' virtual method`

Ask the subclass to allocate a buffer. The subclass decides which size this buffer should be. The default implementation will create a new buffer from the negotiated allocator.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _create_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
create ([GstPushSrc](base/gstpushsrc.html#GstPushSrc "GstPushSrc") * src,
        [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") ** buf)

Ask the subclass to create a buffer. The subclass decides which size this buffer should be. Other then that, refer to[GstBaseSrc](base/gstbasesrc.html#GstBaseSrc).create for more details. If this method is not implemented, _alloc_ followed by _fill_ will be called.

**Parameters:**

__`src`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_create_ 


`function vfunc_create(src: [GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"), buf: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'create' virtual method
}`

Ask the subclass to create a buffer. The subclass decides which size this buffer should be. Other then that, refer to[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc).create for more details. If this method is not implemented, _alloc_ followed by _fill_ will be called.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

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


`def do_create (src, buf):
    #python implementation of the 'create' virtual method`

Ask the subclass to create a buffer. The subclass decides which size this buffer should be. Other then that, refer to[GstBase.BaseSrc](base/gstbasesrc.html#GstBaseSrc).create for more details. If this method is not implemented, _alloc_ followed by _fill_ will be called.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns a tuple made of:**

([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn") )–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer") )–

_No description available_ 

---

#### _fill_ 


[GstFlowReturn](gstreamer/gstpad.html#GstFlowReturn "GstFlowReturn")
fill ([GstPushSrc](base/gstpushsrc.html#GstPushSrc "GstPushSrc") * src,
      [GstBuffer](gstreamer/gstbuffer.html#GstBuffer "GstBuffer") * buf)

Ask the subclass to fill the buffer with data.

**Parameters:**

__`src`_–

_No description available_ 

__`buf`_–

_No description available_ 

**Returns**–

_No description available_ 

---

#### _vfunc\_fill_ 


`function vfunc_fill(src: [GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"), buf: [Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer")): {
    // javascript implementation of the 'fill' virtual method
}`

Ask the subclass to fill the buffer with data.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

#### _do\_fill_ 


`def do_fill (src, buf):
    #python implementation of the 'fill' virtual method`

Ask the subclass to fill the buffer with data.

**Parameters:**

__`src`_ ([GstBase.PushSrc](base/gstpushsrc.html#GstPushSrc "GstBase.PushSrc"))–

_No description available_ 

__`buf`_ ([Gst.Buffer](gstreamer/gstbuffer.html#GstBuffer "Gst.Buffer"))–

_No description available_ 

**Returns** ([Gst.FlowReturn](gstreamer/gstpad.html#GstFlowReturn "Gst.FlowReturn"))–

_No description available_ 

---

The results of the search are