# chromiumsrc - GStreamer Chromium Source Plugin

Headless/Offscreen Chromium browser as a GStreamer video source.

Using CEF Version 145.

## Current State

This is in active development. I'm yet struggling with these tasks:

- Getting the GPU Subprocess to launch properly
- Have a reusable Application, so that there can be multiple Browser instances and even restartable pipeline elements
- Figure out, why the app sometimes crashes
- Reduce the CPU utilization

## Supported Platforms

| Platform | Architecture |
|----------|--------------|
| Linux    | x86_64       |

It could technically work on other systems, but the intention is to have this work as a render-service on a linux
server. It is currently **untested** on any other platform.

## Quick Start

```bash
# Run the setup script (downloads CEF and builds CEF libraries)
./setup_cef.sh

# Build the plugin, installs it to this system and runs a 5 second test pipeline, composing a 
# transparent browserpage above a test-video.
./test.sh
```

### Build manually

```bash
# Configure and build
make

# Install it to this system
make install
```

### Example pipeline

Simple Example, directly outputting to the desktop:

```bash
gst-launch-1.0 chromiumsrc url="https://example.com" ! videoconvert ! autovideosink
```

Composing Example with CPU:

```bash
gst-launch-1.0 compositor name=comp ! videoconvert ! autovideosink \
  videotestsrc ! video/x-raw,format=RGBA,width=1920,height=1080 ! queue ! comp. \
  chromiumsrc url="https://png.ninja/png-test.html" ! videoconvert ! video/x-raw,format=RGBA ! videoscale ! video/x-raw,width=1920,height=1080 ! queue ! comp.
```

Composing Example with OpenGL (GPU):

```bash
gst-launch-1.0 glvideomixer name=mix sink_0::zorder=0 sink_1::zorder=1 ! glimagesink 
  videotestsrc ! video/x-raw,width=1920,height=1080,framerate=30/1 ! glupload ! glcolorconvert ! mix. 
  chromiumsrc url="https://png.ninja/png-test.html" width=1920 height=1080 ! glupload ! glcolorconvert ! mix.
```

## Architecture

```
gst-launch-1.0 chromiumsrc url="https://example.com" ! videoconvert ! autovideosink
                   │
                   ▼
           ┌─────────────────┐
           │ GstChromiumSrc  │  (GstBin subclass)
           │                 │
           │  ┌───────┐      │
           │  │appsrc │      │  ← pull mode via need-data signal
           │  └───┬───┘      │
           └──────┼──────────┘
                  │
                  │  shared memory
                  │
                  ▼
           ┌─────────────┐
           │ CEF Browser │  (offscreen, windowless)
           │             │
           │ OnPaint()   │  → BGRA frames → buffer
           └─────────────┘
```

## Files

| File                             | Purpose                                                         |
|----------------------------------|-----------------------------------------------------------------|
| `gstchromiumsrc.h`               | GstChromiumSrc type definitions                                 |
| `gstchromiumsrc.cpp`             | GStreamer element: properties, state changes, need-data handler |
| `cef_render_handler.h`           | CEF handler declarations                                        |
| `cef_render_handler.cpp`         | CEF integration: browser lifecycle, OnPaint → frame buffer      |
| `gpu_utils.h`                    | GPU detection and configuration API                             |
| `gpu_utils.cpp`                  | GPU detection: render node discovery, auto-select best GPU      |
| `Makefile`                       | Build configuration                                             |
| `setup_cef.sh`                   | Setup script: dependency check, CEF download, wrapper build     |
| `third_party/cef/`               | CEF browser files (downloaded)                                  |
| `third_party/build_cef_wrapper/` | CEF wrapper build dir (copies to cef/Release/)                  |

## Data Flow

1. **GST→CEF**: `need-data` signal → wait on `frame_cond`
2. **CEF→GST**: `OnPaint()` → copy BGRA to `frame_buffer` → signal `frame_cond`
3. **Frame push**: `need-data` handler → wrap buffer → `gst_app_src_push_buffer()`

## Properties

| Property    | Type   | Default                         | Description                         |
|-------------|--------|---------------------------------|-------------------------------------|
| `url`       | string | `https://example.com/test.html` | URL to render                       |
| `width`     | int    | 1920                            | Video width in px                   |
| `height`    | int    | 1080                            | Video height in px                  |
| `framerate` | string | `30`                            | Output framerate (recommended `30`) |
| `gpu`       | string | `auto`                          | GPU: `auto`, `true`, `false`        |

## GPU Acceleration

The `gpu` property controls GPU acceleration for offscreen rendering:

- `auto` (default): Auto-detect GPU.
- `true`: Force GPU mode. Uses EGL (ANGLE) for hardware-accelerated rendering.
- `false`: Disable GPU. Uses software/CPU rendering (swiftshader).

**GPU headless mode** (server without display):

```bash
gst-launch-1.0 chromiumsrc url="https://png.ninja/png-test.html" gpu=true ! videoconvert ! autovideosink
```

**Requirements for GPU on Linux:**

- `/dev/dri/renderD*` device (GPU render node)
- `libegl1`, `libgbm1` installed
- User in `render` or `video` group

## Output Format

BGRA only (native CEF offscreen format with alpha channel).

## Known Issues

- CEF subprocess handling requires `--single-process` mode in plugin context
- First frame may be black if captured before page load completes (unnoticable)
- **macOS**: CEF requires a GUI context even for offscreen rendering. Headless operation may need additional setup
  (e.g., running with a display server or in a logged-in GUI session)
