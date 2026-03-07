#!/bin/bash

# Install Rust and Cargo-C if needed:
#   curl https://sh.rustup.rs -sSf | sh
#   cargo install cargo-c

git clone https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs.git || echo "Already downloaded"
cd ./gst-plugins-rs || exit

# Check if cargo is installed
if ! command -v cargo &> /dev/null
then
    echo ""
    echo "Cargo could not be found. Please install Rust and Cargo before running this script."
    echo "To install Cargo, run:"
    echo "    curl https://sh.rustup.rs -sSf | sh"
    echo "Then please install cargo-c:"
    echo "    cargo install cargo-c"
    echo ""
    exit
fi

git checkout 0.15.1

cargo cinstall -p gst-plugin-livesync --prefix=./dist/
cargo cinstall -p gst-plugin-fallbackswitch --prefix=./dist/
cargo cinstall -p gst-plugin-togglerecord --prefix=./dist/
cargo cinstall -p gst-plugin-deepgram --prefix=./dist/
cargo cinstall -p gst-plugin-mpegtslive --prefix=./dist/
cargo cinstall -p gst-plugin-tracers --prefix=./dist/
cargo cinstall -p gst-plugin-uriplaylistbin --prefix=./dist/
cargo cinstall -p gst-plugin-debugseimetainserter --prefix=./dist/

echo "Installing into system /usr/local/lib/"
sudo cp -r dist/lib/x86_64-linux-gnu/* /usr/local/lib/

echo "Done. The following plugins were installed:"
echo "   - gst-plugin-livesync"
echo "   - gst-plugin-fallbackswitch"
echo "   - gst-plugin-togglerecord"
echo "   - gst-plugin-deepgram"
echo "   - gst-plugin-mpegtslive"
echo "   - gst-plugin-tracers"
echo "   - gst-plugin-uriplaylistbin"
echo "   - gst-plugin-debugseimetainserter"
echo ""
