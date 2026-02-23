CEF_DIR = third_party/cef
CEF_LIB_DIR = $(CEF_DIR)/Release
CEF_LIB = $(CEF_LIB_DIR)/libcef.so
CEF_WRAPPER = build_cef_wrapper/libcef_dll_wrapper/libcef_dll_wrapper.a

GST_CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)
GST_LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)

GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS = $(shell pkg-config --libs glib-2.0)

CXXFLAGS = -std=c++20 -fPIC -shared -O2 -fpermissive \
    -I$(CEF_DIR) \
    $(GST_CFLAGS) \
    $(GLIB_CFLAGS) \
    -DPACKAGE=\"cefsrc\" \
    -DPACKAGE_VERSION=\"1.0.0\"

LDFLAGS = $(CEF_WRAPPER) -L$(CEF_LIB_DIR) -lcef \
    $(GST_LIBS) \
    $(GLIB_LIBS) \
    -Wl,-rpath,'$$ORIGIN/../$(CEF_LIB_DIR)'

SOURCES = gstcefsrc.cpp cef_render_handler.cpp

PLUGIN = libgstcefsrc.so

.PHONY: all clean install

all: $(PLUGIN)

$(PLUGIN): $(SOURCES) gstcefsrc.h cef_render_handler.h
	g++ $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

install: $(PLUGIN)
	mkdir -p ~/.local/share/gstreamer-1.0/plugins
	cp $(PLUGIN) ~/.local/share/gstreamer-1.0/plugins/

clean:
	rm -f $(PLUGIN)

test: $(PLUGIN)
	GST_PLUGIN_PATH=. gst-launch-1.0 cefsrc url="https://pingup.de/w/png-test.html" width=1920 height=1080 ! videoconvert ! autovideosink
