# cefsrc - GStreamer CEF Source Plugin

Offscreen Chromium browser as a GStreamer video source using CEF (Chromium Embedded Framework).

## Architecture

```
gst-launch-1.0 cefsrc url="https://example.com" ! videoconvert ! autovideosink
                    │
                    ▼
              ┌─────────────┐
              │  GstCefSrc  │  (GstBin subclass)
              │             │
              │  ┌───────┐  │
              │  │appsrc │  │  ← pull mode via need-data signal
              │  └───┬───┘  │
              └──────┼──────┘
                     │
                     ▼
              ┌─────────────┐
              │ CEF Browser │  (offscreen, windowless)
              │             │
              │ OnPaint()   │  → BGRA frames → appsrc
              └─────────────┘
```

## Files

| File                     | Purpose                                                         |
|--------------------------|-----------------------------------------------------------------|
| `gstcefsrc.h`            | GstCefSrc type definitions (GstBin + appsrc + CEF state)        |
| `gstcefsrc.cpp`          | GStreamer element: properties, state changes, need-data handler |
| `cef_render_handler.h`   | CEF handler declarations                                        |
| `cef_render_handler.cpp` | CEF integration: browser lifecycle, OnPaint → frame buffer      |
| `Makefile`               | Build configuration                                             |
| `third_party/cef/`       | CEF distribution                                                |
| `build_cef_wrapper/`     | Compiled libcef_dll_wrapper.a                                   |

## Data Flow

1. **GST→CEF**: `need-data` signal → wait on `frame_cond`
2. **CEF→GST**: `OnPaint()` → copy BGRA to `frame_buffer` → signal `frame_cond`
3. **Frame push**: `need-data` handler → wrap buffer → `gst_app_src_push_buffer()`

## Build

```bash
# Build CEF wrapper (once)
mkdir -p build_cef_wrapper && cd build_cef_wrapper
cmake ../third_party/cef -DCMAKE_BUILD_TYPE=Release
make libcef_dll_wrapper -j$(nproc)
cd ..

# Build plugin
make
```

## Properties

| Property    | Type    | Default                             | Description                     |
|-------------|---------|-------------------------------------|---------------------------------|
| `url`       | string  | `https://pingup.de/w/png-test.html` | URL to render                   |
| `width`     | int     | 1920                                | Video width                     |
| `height`    | int     | 1080                                | Video height                    |
| `framerate` | string  | `1/1`                               | Output framerate (e.g., `30/1`) |

## Output Format

BGRA only (native CEF offscreen format with alpha channel).

## Known Issues

- CEF subprocess handling requires `--single-process` mode in plugin context
- First frame may be black if captured before page load completes
