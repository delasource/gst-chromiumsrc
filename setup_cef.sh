#!/bin/bash
set -e

CEF_VERSION="145.0.26%2Bg6ed7554%2Bchromium-145.0.7632.110"
CEF_DIR="third_party/cef"
BUILD_DIR="build_cef_wrapper"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$SCRIPT_DIR"

detect_platform() {
    OS="$(uname -s)"
    ARCH="$(uname -m)"
    
    case "$OS" in
        Linux)
            PLATFORM="linux64"
            CEF_PLATFORM="linux64"
            ;;
        Darwin)
            if [ "$ARCH" = "arm64" ]; then
                PLATFORM="macosarm64"
                CEF_PLATFORM="macosarm64"
            else
                PLATFORM="macosx64"
                CEF_PLATFORM="macosx64"
            fi
            ;;
        *)
            echo "Unsupported platform: $OS $ARCH"
            exit 1
            ;;
    esac
    
    echo "Detected platform: $PLATFORM"
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        return 1
    fi
    return 0
}

check_linux_deps() {
    local missing=()
    
    for cmd in cmake curl make pkg-config; do
        if ! check_command "$cmd"; then
            missing+=("$cmd")
        fi
    done
    
    if ! check_command "g++" && ! check_command "clang++"; then
        missing+=("g++ (or clang++)")
    fi
    
    if ! pkg-config --exists gstreamer-1.0 2>/dev/null; then
        missing+=("libgstreamer1.0-dev")
    fi
    
    if ! pkg-config --exists glib-2.0 2>/dev/null; then
        missing+=("libglib2.0-dev")
    fi
    
    echo "${missing[@]}"
}

check_macos_deps() {
    local missing=()
    
    if ! check_command "brew"; then
        missing+=("homebrew")
    fi
    
    for cmd in cmake curl make pkg-config; do
        if ! check_command "$cmd"; then
            missing+=("$cmd")
        fi
    done
    
    if ! check_command "g++" && ! check_command "clang++"; then
        missing+=("clang++ (Xcode command line tools)")
    fi
    
    if ! brew list gstreamer &>/dev/null 2>&1; then
        missing+=("gstreamer")
    fi
    
    if ! brew list glib &>/dev/null 2>&1; then
        missing+=("glib")
    fi
    
    echo "${missing[@]}"
}

install_linux_deps() {
    local deps=("$@")
    
    if [ ${#deps[@]} -eq 0 ]; then
        return 0
    fi
    
    echo "Missing dependencies: ${deps[*]}"
    read -p "Install them now? [y/N] " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled. Please install dependencies manually."
        exit 1
    fi
    
    if check_command "apt-get"; then
        sudo apt-get update
        sudo apt-get install -y cmake curl build-essential pkg-config \
            libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
            libgstreamer-plugins-good1.0-dev libgstreamer-plugins-bad1.0-dev \
            libglib2.0-dev
    elif check_command "dnf"; then
        sudo dnf install -y cmake curl gcc-c++ make pkgconfig \
            gstreamer1-devel gstreamer1-plugins-base-devel \
            glib2-devel
    elif check_command "pacman"; then
        sudo pacman -S --noconfirm cmake curl base-devel pkg-config \
            gstreamer gst-plugins-base gst-plugins-good glib2
    else
        echo "Unsupported package manager. Please install dependencies manually:"
        echo "  - cmake, curl, make, gcc/g++, pkg-config"
        echo "  - GStreamer development packages"
        echo "  - GLib development packages"
        exit 1
    fi
}

install_macos_deps() {
    local deps=("$@")
    
    if [ ${#deps[@]} -eq 0 ]; then
        return 0
    fi
    
    if [[ " ${deps[*]} " =~ " homebrew " ]]; then
        echo "Homebrew is required but not installed."
        echo "Install it from: https://brew.sh"
        exit 1
    fi
    
    echo "Missing dependencies: ${deps[*]}"
    read -p "Install them now? [y/N] " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled. Please install dependencies manually."
        exit 1
    fi
    
    for dep in "${deps[@]}"; do
        case "$dep" in
            "clang++ (Xcode command line tools)")
                echo "Installing Xcode command line tools..."
                xcode-select --install 2>/dev/null || true
                ;;
            *)
                echo "Installing $dep via Homebrew..."
                brew install "$dep"
                ;;
        esac
    done
}

download_cef() {
    local cef_url="https://cef-builds.spotifycdn.com/cef_binary_${CEF_VERSION}_${CEF_PLATFORM}.tar.bz2"
    
    if [ -d "$CEF_DIR" ]; then
        echo "CEF already exists at $CEF_DIR"
        read -p "Re-download? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            return 0
        fi
        rm -rf "$CEF_DIR"
    fi
    
    echo "Downloading CEF for $CEF_PLATFORM..."
    mkdir -p "$CEF_DIR"
    curl -L "$cef_url" -o /tmp/cef.tar.bz2
    
    echo "Extracting CEF..."
    tar -xjf /tmp/cef.tar.bz2 -C "$CEF_DIR" --strip-components=1
    
    rm /tmp/cef.tar.bz2
    
    echo "CEF installed to $CEF_DIR"
}

build_wrapper() {
    if [ -f "$BUILD_DIR/libcef_dll_wrapper/libcef_dll_wrapper.a" ]; then
        echo "CEF wrapper already built at $BUILD_DIR/libcef_dll_wrapper/libcef_dll_wrapper.a"
        read -p "Rebuild? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            return 0
        fi
    fi
    
    echo "Building CEF wrapper..."
    
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake ../third_party/cef -DCMAKE_BUILD_TYPE=Release
    
    if [ "$(uname -s)" = "Darwin" ]; then
        NPROC=$(sysctl -n hw.ncpu)
    else
        NPROC=$(nproc)
    fi
    
    make libcef_dll_wrapper -j"$NPROC"
    
    cd "$SCRIPT_DIR"
    
    echo "CEF wrapper built successfully"
}

main() {
    echo "=== CEF Setup Script ==="
    echo
    
    detect_platform
    
    echo
    echo "Checking dependencies..."
    
    if [ "$(uname -s)" = "Darwin" ]; then
        missing=$(check_macos_deps)
        install_macos_deps $missing
    else
        missing=$(check_linux_deps)
        install_linux_deps $missing
    fi
    
    echo
    echo "All dependencies satisfied."
    
    echo
    download_cef
    
    echo
    build_wrapper
    
    echo
    echo "=== Setup Complete ==="
    echo "You can now run 'make' to build the plugin."
}

main "$@"
