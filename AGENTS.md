# Build Commands

run:

```
make clean && make
```

test it:

```
make install

# If a display is available:
timeout 5 gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" ! videoconvert ! autovideosink

# else:
gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" ! videoconvert ! \
  x264enc tune=zerolatency ! video/x-h264, profile=high ! queue ! \
  mpegtsmux ! srtsink uri="srt://localhost:8890?streamid=publish:inproc:dela_rs:j1olzl7o&pktsize=1316" latency=2000 sync=false async=false

```

# CEF Documentation

Find CEF instructions here: https://raw.githubusercontent.com/chromiumembedded/cef/refs/heads/master/tools/claude/CLAUDE_CLIENT_INSTRUCTIONS.md
