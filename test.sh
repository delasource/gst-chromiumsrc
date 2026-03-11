#!/bin/bash
rm -rf build
cmake -B build -DCMAKE_BUILD_TYPE=Release || exit
cmake --build build || exit
cmake --install build

#timeout 5 \
#gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" \
#  ! videoconvert ! x264enc tune=zerolatency ! video/x-h264, profile=high ! queue ! mpegtsmux \
#  ! srtsink uri="srt://localhost:8890?streamid=publish:inproc:dela_rs:j1olzl7o&pktsize=1316" latency=2000 sync=false async=false

#timeout 5 \
#gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" \
#  ! videoconvert ! queue ! autovideosink

# Using Compositor and CPU rendering
#timeout 6 \
#gst-launch-1.0 compositor name=comp ! videoconvert ! autovideosink \
#  videotestsrc ! video/x-raw,format=RGBA ! videoscale ! video/x-raw,width=1920,height=1080 ! queue ! comp. \
#  chromiumsrc url="https://png.ninja/png-test.html" disable-gpu=1 ! videoconvert ! video/x-raw,format=RGBA ! videoscale ! video/x-raw,width=1920,height=1080 ! queue ! comp.

# Using compositor and GPU AUTO
timeout 6 \
gst-launch-1.0 compositor name=comp ! videoconvert ! autovideosink \
  videotestsrc ! video/x-raw,format=RGBA,width=1920,height=1080 ! queue ! comp. \
  chromiumsrc url="https://png.ninja/png-test.html" ! videoconvert ! video/x-raw,format=RGBA ! videoscale ! video/x-raw,width=1920,height=1080 ! queue ! comp.

# Using glvideomixer
#timeout 6 \
#gst-launch-1.0 glvideomixer name=mix ! glimagesink \
#  videotestsrc ! video/x-raw,format=RGBA,width=1920,height=1080 ! glupload ! glcolorconvert ! queue ! mix. \
#  chromiumsrc url="https://png.ninja/png-test.html" ! videoconvert ! video/x-raw,format=RGBA ! videoscale ! video/x-raw,width=1920,height=1080 ! glupload ! glcolorconvert ! queue ! mix.

