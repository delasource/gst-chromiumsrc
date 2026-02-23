#!/bin/bash
set -e

CEF_URL="https://cef-builds.spotifycdn.com/cef_binary_145.0.26%2Bg6ed7554%2Bchromium-145.0.7632.110_linux64.tar.bz2"
CEF_DIR="third_party/cef"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$SCRIPT_DIR"

if [ -d "$CEF_DIR" ]; then
    echo "CEF already exists at $CEF_DIR"
    exit 0
fi

echo "Downloading CEF..."
mkdir -p $CEF_DIR
curl -L "$CEF_URL" -o /tmp/cef.tar.bz2

echo "Extracting CEF..."
mkdir -p "$CEF_DIR"
tar -xjf /tmp/cef.tar.bz2 -C $CEF_DIR --strip-components=1

rm /tmp/cef.tar.bz2

echo "CEF installed to $CEF_DIR"
