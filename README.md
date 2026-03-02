# chromiumsrc - GStreamer Chromium Source Plugin

Offscreen Chromium browser as a GStreamer video source using CEF (Chromium Embedded Framework).

## Supported Platforms

| Platform | Architecture          |
|----------|-----------------------|
| Linux    | x86_64                |
| macOS    | arm64 (Apple Silicon) |
| macOS    | x86_64 (Intel)        |

## Quick Start

```bash
# Run the setup script (downloads CEF and builds CEF libraries)
./setup_cef.sh

# Build the plugin
make

# Install the builded plugin to the system
make install
```

### Example pipeline

```bash
gst-launch-1.0 chromiumsrc url="https://example.com" ! videoconvert ! autovideosink
```

```bash
gst-launch-1.0 glvideomixer name=mix sink_0::zorder=0 sink_1::zorder=1 ! glimagesink 
  videotestsrc ! video/x-raw,width=1920,height=1080,framerate=30/1 ! glupload ! glcolorconvert ! mix. 
  chromiumsrc url="https://pingup.de/w/png-test.html" width=1920 height=1080 ! glupload ! glcolorconvert ! mix.
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

| File                     | Purpose                                                         |
|--------------------------|-----------------------------------------------------------------|
| `gstchromiumsrc.h`       | GstChromiumSrc type definitions                                 |
| `gstchromiumsrc.cpp`     | GStreamer element: properties, state changes, need-data handler |
| `cef_render_handler.h`   | CEF handler declarations                                        |
| `cef_render_handler.cpp` | CEF integration: browser lifecycle, OnPaint → frame buffer      |
| `gpu_utils.h`            | GPU detection and configuration API                             |
| `gpu_utils.cpp`          | GPU detection: render node discovery, auto-select best GPU      |
| `Makefile`               | Build configuration                                             |
| `setup_cef.sh`           | Setup script: dependency check, CEF download, wrapper build     |
| `third_party/cef/`       | CEF browser files (downloaded)                                  |
| `build_cef_wrapper/`     | CEF C-library compiled 'libcef_dll_wrapper.a'                   |

## Data Flow

1. **GST→CEF**: `need-data` signal → wait on `frame_cond`
2. **CEF→GST**: `OnPaint()` → copy BGRA to `frame_buffer` → signal `frame_cond`
3. **Frame push**: `need-data` handler → wrap buffer → `gst_app_src_push_buffer()`

## Properties

| Property    | Type   | Default                         | Description                   |
|-------------|--------|---------------------------------|-------------------------------|
| `url`       | string | `https://example.com/test.html` | URL to render                 |
| `width`     | int    | 1920                            | Video width                   |
| `height`    | int    | 1080                            | Video height                  |
| `framerate` | string | `30`                            | Output framerate (e.g., `30`) |
| `gpu`       | string | `auto`                          | GPU: `auto`, `true`, `false`  |

Please note that adjusting the framerate here will not limit the framerate (animation frame time) of the browser or
javascript. That will be 60fps nevertheless.

## GPU Acceleration

The `gpu` property controls GPU acceleration for offscreen rendering:

- `auto` (default): Auto-detect GPU. Enables GPU if `/dev/dri/renderD*` devices exist.
- `true`: Force GPU mode. Uses EGL for hardware-accelerated rendering.
- `false`: Disable GPU. Uses software/CPU rendering.

**GPU headless mode** (server without display):
```bash
gst-launch-1.0 chromiumsrc url="https://example.com" gpu=true ! videoconvert ! autovideosink
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
