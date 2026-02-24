CEF_DIR = third_party/cef
CEF_LIB_DIR = $(CEF_DIR)/Release
CEF_WRAPPER = build_cef_wrapper/libcef_dll_wrapper/libcef_dll_wrapper.a

UNAME_S := $(shell uname -s)

GST_CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)
GST_LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)

GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS = $(shell pkg-config --libs glib-2.0)

ifeq ($(UNAME_S),Darwin)
	CEF_FRAMEWORK_NAME = Chromium Embedded Framework.framework
	PLUGIN = libgstchromiumsrc.dylib
	CXXFLAGS = -std=c++20 -fPIC -shared -O2 \
		-I$(CEF_DIR) \
		$(GST_CFLAGS) \
		$(GLIB_CFLAGS) \
		-DPACKAGE=\"chromiumsrc\" \
		-DPACKAGE_VERSION=\"1.0.0\" \
		-stdlib=libc++
	LDFLAGS = $(CEF_WRAPPER) \
		-F$(CEF_LIB_DIR) \
		-framework "Chromium Embedded Framework" \
		$(GST_LIBS) \
		$(GLIB_LIBS) \
		-framework AppKit \
		-framework Cocoa \
		-framework IOSurface \
		-Wl,-rpath,@loader_path
	INSTALL_DIR = $(HOME)/Library/GStreamer/1.0/plugins
else
	CEF_LIB = $(CEF_LIB_DIR)/libcef.so
	PLUGIN = libgstchromiumsrc.so
	CXXFLAGS = -std=c++20 -fPIC -shared -O2 -fpermissive \
		-I$(CEF_DIR) \
		$(GST_CFLAGS) \
		$(GLIB_CFLAGS) \
		-DPACKAGE=\"chromiumsrc\" \
		-DPACKAGE_VERSION=\"1.0.0\"
	LDFLAGS = $(CEF_WRAPPER) -L$(CEF_LIB_DIR) -lcef \
		$(GST_LIBS) \
		$(GLIB_LIBS) \
		-Wl,-rpath,'$$ORIGIN'
	INSTALL_DIR = $(HOME)/.local/share/gstreamer-1.0/plugins
endif

SOURCES = gstchromiumsrc.cpp cef_render_handler.cpp gpu_utils.cpp

.PHONY: all clean install

all: $(PLUGIN)

$(PLUGIN): $(SOURCES) gstchromiumsrc.h cef_render_handler.h gpu_utils.h
	g++ $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

ifeq ($(UNAME_S),Darwin)
install: $(PLUGIN)
	mkdir -p "$(INSTALL_DIR)"
	cp "$(PLUGIN)" "$(INSTALL_DIR)/"
	cp -R "$(CEF_LIB_DIR)/$(CEF_FRAMEWORK_NAME)" "$(INSTALL_DIR)/"
	install_name_tool -change "@executable_path/../Frameworks/Chromium Embedded Framework.framework/Chromium Embedded Framework" "@loader_path/Chromium Embedded Framework.framework/Chromium Embedded Framework" "$(INSTALL_DIR)/$(PLUGIN)"
else
CEF_LIBS = $(CEF_LIB_DIR)/libcef.so $(CEF_LIB_DIR)/libEGL.so $(CEF_LIB_DIR)/libGLESv2.so $(CEF_LIB_DIR)/libvulkan.so.1 $(CEF_LIB_DIR)/libvk_swiftshader.so



install: $(PLUGIN)
	mkdir -p "$(INSTALL_DIR)"
	cp $(PLUGIN) "$(INSTALL_DIR)/"
	cp $(CEF_LIBS) "$(INSTALL_DIR)/"
	cp -r "$(CEF_DIR)/Resources/*" "$(INSTALL_DIR)"
	cp -r "$(CEF_DIR)/Release/*" "$(INSTALL_DIR)"
endif

clean:
	rm -f $(PLUGIN)

test: $(PLUGIN)
	GST_PLUGIN_PATH=. gst-launch-1.0 chromiumsrc url="https://pingup.de/w/png-test.html" width=1920 height=1080 ! videoconvert ! autovideosink
