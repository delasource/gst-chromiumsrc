# Makefile for gst-chromiumsrc GStreamer Plugin
#
# This Makefile builds two artifacts:
#   1. libgstchromiumsrc.so/dylib - GStreamer plugin for Chromium-based video source
#   2. chromiumsrc-subprocess     - CEF subprocess binary for multi-process architecture
#
# Build targets:
#   make           - Builds both the plugin and subprocess binary
#   make install   - Installs both artifacts to the GStreamer plugin directory
#   make clean     - Removes built artifacts
#   make test      - Runs a basic test pipeline
#
# Prerequisites:
#   - CEF (Chromium Embedded Framework) in third_party/cef/
#   - Built CEF wrapper library at build_cef_wrapper/libcef_dll_wrapper/
#   - GStreamer 1.0 development libraries
#   - GLib 2.0 development libraries
#
# Cross-platform support:
#   - Linux: Builds .so plugin, uses -lcef linking
#   - macOS: Builds .dylib plugin, uses framework linking

# CEF Configuration
CEF_DIR = third_party/cef
CEF_LIB_DIR = $(CEF_DIR)/Release
CEF_WRAPPER = build_cef_wrapper/libcef_dll_wrapper/libcef_dll_wrapper.a

UNAME_S := $(shell uname -s)

# Output Directory
DIST_DIR = dist

# GStreamer and GLib Configuration
GST_CFLAGS = $(shell pkg-config --cflags gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)
GST_LIBS = $(shell pkg-config --libs gstreamer-1.0 gstreamer-app-1.0 gstreamer-base-1.0)

GLIB_CFLAGS = $(shell pkg-config --cflags glib-2.0)
GLIB_LIBS = $(shell pkg-config --libs glib-2.0)

# Platform-Specific Configuration
ifeq ($(UNAME_S),Darwin)
	# macOS Configuration
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
endif

# Subprocess Binary Linker Flags
#
# The subprocess binary (chromiumsrc-subprocess) is a standalone executable
# that handles CEF's renderer, GPU, and utility processes. It needs to link
# against CEF but NOT against GStreamer (which is only needed by the plugin).
ifeq ($(UNAME_S),Darwin)
	SUBPROCESS_LDFLAGS = $(CEF_WRAPPER) \
		-F$(CEF_LIB_DIR) \
		-framework "Chromium Embedded Framework" \
		$(GLIB_LIBS) \
		-Wl,-rpath,@loader_path
else
	SUBPROCESS_LDFLAGS = $(CEF_WRAPPER) -L$(CEF_LIB_DIR) -lcef \
		$(GLIB_LIBS) \
		-Wl,-rpath,'$$ORIGIN'
endif

# Build Targets
SOURCES = gstchromiumsrc.cpp cef_render_handler.cpp gpu_utils.cpp
SUBPROCESS = chromiumsrc-subprocess

.PHONY: all clean install cef-binaries

# Build both the GStreamer plugin and the subprocess binary
all: $(DIST_DIR)/$(PLUGIN) $(DIST_DIR)/$(SUBPROCESS)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

# GStreamer Plugin Build Rule
#
# Builds the shared library that GStreamer loads as a source element.
# This plugin initializes CEF and manages the browser lifecycle.
$(DIST_DIR)/$(PLUGIN): $(SOURCES) gstchromiumsrc.h cef_render_handler.h gpu_utils.h | $(DIST_DIR)
	g++ $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

# CEF Subprocess Binary Build Rule
#
# Builds the standalone executable for CEF's multi-process architecture.
# When CEF needs to spawn a renderer or GPU process, it executes this binary
# with special command-line flags. The binary then calls CefExecuteProcess()
# to handle the subprocess logic.
#
# See main.cpp for detailed documentation of the subprocess architecture.
$(DIST_DIR)/$(SUBPROCESS): main.cpp | $(DIST_DIR)
	g++ -std=c++20 -O2 \
		-I$(CEF_DIR) \
		$(GLIB_CFLAGS) \
		-o $@ main.cpp \
		$(SUBPROCESS_LDFLAGS)

# Copy CEF binaries to dist folder
$(DIST_DIR)/.cef-copied: | $(DIST_DIR)
	cp -r "$(CEF_DIR)/Resources/"* "$(DIST_DIR)/"
	cp -r "$(CEF_DIR)/Release/"* "$(DIST_DIR)/"
	touch $@

cef-binaries: $(DIST_DIR)/.cef-copied

install: $(DIST_DIR)/$(PLUGIN) $(DIST_DIR)/$(SUBPROCESS)
	mkdir -p "$(INSTALL_DIR)"
	cp $(DIST_DIR)/$(PLUGIN) "$(INSTALL_DIR)/"
	cp $(DIST_DIR)/$(SUBPROCESS) "$(INSTALL_DIR)/"
	cp -r "$(CEF_DIR)/Resources/"* "$(INSTALL_DIR)/"
	cp -r "$(CEF_DIR)/Release/"* "$(INSTALL_DIR)/"

# Clean Rule
clean:
	rm -r $(DIST_DIR)

# Test Rule
#
# Runs a basic test pipeline to verify the plugin works correctly.
# Opens a test URL and displays it using autovideosink.
test: $(DIST_DIR)/$(PLUGIN)
	GST_PLUGIN_PATH=$(DIST_DIR) gst-launch-1.0 chromiumsrc url="https://pingup.de/w/png-test.html" width=1920 height=1080 ! videoconvert ! autovideosink
