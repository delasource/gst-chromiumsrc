# Makefile for gst-chromiumsrc GStreamer Plugin
#
# This Makefile builds two artifacts:
#   1. libgstchromiumsrc.so   - GStreamer plugin for Chromium-based video source
#   2. chromiumsrc-subprocess - CEF subprocess binary for multi-process architecture
#
# Build targets:
#   make           - Builds both the plugin and subprocess binary
#   make install   - Installs both artifacts to the GStreamer plugin directory
#   make clean     - Removes built artifacts
#   make test      - Runs a basic test pipeline

# CEF Configuration
CEF_DIR = third_party/cef
CEF_LIB_DIR = $(CEF_DIR)/Release
CEF_WRAPPER = build_cef_wrapper/libcef_dll_wrapper/libcef_dll_wrapper.a

UNAME_S := $(shell uname -s)


# GStreamer and GLib Configuration
GST_CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)
GST_LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)

GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS = $(shell pkg-config --libs glib-2.0)

# Linux Configuration
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

# Subprocess Binary Linker Flags
#
# The subprocess binary (chromiumsrc-subprocess) is a standalone executable
# that handles CEF's renderer, GPU, and utility processes. It needs to link
# against CEF but NOT against GStreamer (which is only needed by the plugin).
SUBPROCESS_LDFLAGS = $(CEF_WRAPPER) -L$(CEF_LIB_DIR) -lcef \
	$(GLIB_LIBS) \
	-Wl,-rpath,'$$ORIGIN'


# Build Targets
SOURCES = gstchromiumsrc.cpp cef_render_handler.cpp gpu_utils.cpp
SUBPROCESS = chromiumsrc-subprocess

.PHONY: all clean install

# Build both the GStreamer plugin and the subprocess binary
all: $(PLUGIN) $(SUBPROCESS)

# GStreamer Plugin Build Rule
#
# Builds the shared library that GStreamer loads as a source element.
# This plugin initializes CEF and manages the browser lifecycle.
$(PLUGIN): $(SOURCES) gstchromiumsrc.h cef_render_handler.h gpu_utils.h
	g++ $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

# CEF Subprocess Binary Build Rule
#
# Builds the standalone executable for CEF's multi-process architecture.
# When CEF needs to spawn a renderer or GPU process, it executes this binary
# with special command-line flags. The binary then calls CefExecuteProcess()
# to handle the subprocess logic.
#
# See main.cpp for detailed documentation of the subprocess architecture.

$(SUBPROCESS): main.cpp
	g++ -std=c++20 -O2 \
		-I$(CEF_DIR) \
		$(GLIB_CFLAGS) \
		-o $@ main.cpp \
		$(SUBPROCESS_LDFLAGS)

install: $(PLUGIN) $(SUBPROCESS)
	mkdir -p "$(INSTALL_DIR)"
	cp $(PLUGIN) "$(INSTALL_DIR)/"
	cp $(SUBPROCESS) "$(INSTALL_DIR)/"
	cp -r "$(CEF_DIR)/Resources/"* "$(INSTALL_DIR)/"
	cp -r "$(CEF_DIR)/Release/"* "$(INSTALL_DIR)/"

# Clean Rule
clean:
	rm -f $(PLUGIN) $(SUBPROCESS)

# Test Rule
#
# Runs a basic test pipeline to verify the plugin works correctly.
# Opens a test URL and displays it using autovideosink.
test: $(PLUGIN)
	GST_PLUGIN_PATH=. gst-launch-1.0 chromiumsrc url="https://pingup.de/w/png-test.html" width=1920 height=1080 ! videoconvert ! autovideosink
