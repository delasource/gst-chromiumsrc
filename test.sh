#!/bin/bash
make clean
make
make install

#timeout 5 \
#gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" \
#  ! videoconvert ! x264enc tune=zerolatency ! video/x-h264, profile=high ! queue ! mpegtsmux \
#  ! srtsink uri="srt://localhost:8890?streamid=publish:inproc:dela_rs:j1olzl7o&pktsize=1316" latency=2000 sync=false async=false

timeout 5 \
gst-launch-1.0 chromiumsrc url="http://localhost:5173/png-test.html" \
  ! videoconvert ! autovideosink

