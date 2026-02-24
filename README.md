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

Please note that adjusting the framerate here will not limit the framerate (animation frame time) of the browser or
javascript. That will be 60fps nevertheless.

## Output Format

BGRA only (native CEF offscreen format with alpha channel).

## Known Issues

- CEF subprocess handling requires `--single-process` mode in plugin context
- First frame may be black if captured before page load completes (unnoticable)
- **macOS**: CEF requires a GUI context even for offscreen rendering. Headless operation may need additional setup
  (e.g., running with a display server or in a logged-in GUI session)
