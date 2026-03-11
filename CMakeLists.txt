cmake_minimum_required(VERSION 3.21)
project(gst-chromiumsrc CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directory for built artifacts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Platform detection
if(APPLE)
  set(OS_MAC TRUE)
  set(OS_POSIX TRUE)
elseif(UNIX)
  set(OS_LINUX TRUE)
  set(OS_POSIX TRUE)
else()
  message(FATAL_ERROR "Unsupported platform. Only macOS and Linux are supported.")
endif()

# CEF Configuration (pre-built)
set(CEF_DIR "${CMAKE_SOURCE_DIR}/third_party/cef")
set(CEF_INCLUDE_DIR "${CEF_DIR}")
set(CEF_WRAPPER_LIB "${CEF_DIR}/Release/libcef_dll_wrapper.a")

if(OS_MAC)
  set(CEF_FRAMEWORK "${CEF_DIR}/Release/Chromium Embedded Framework.framework")
  set(CEF_FRAMEWORK_DIR "${CEF_DIR}/Release/Chromium Embedded Framework.framework")
  if(NOT EXISTS "${CEF_FRAMEWORK}")
    message(FATAL_ERROR "CEF Framework not found at: ${CEF_FRAMEWORK}")
  endif()
elseif(OS_LINUX)
  set(CEF_LIB_DIR "${CEF_DIR}/Release")
  set(CEF_LIB "${CEF_LIB_DIR}/libcef.so")
  if(NOT EXISTS "${CEF_LIB}")
    message(FATAL_ERROR "CEF library not found at: ${CEF_LIB}")
  endif()
endif()

if(NOT EXISTS "${CEF_WRAPPER_LIB}")
  message(FATAL_ERROR "CEF wrapper library not found at: ${CEF_WRAPPER_LIB}")
endif()

# GStreamer and GLib via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
  gstreamer-1.0
  gstreamer-app-1.0
  gstreamer-base-1.0
)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Common compile definitions
set(COMMON_DEFINES
  PACKAGE="chromiumsrc"
  PACKAGE_VERSION="1.0.0"
)

# ============================================================================
# GStreamer Plugin Target
# ============================================================================
set(PLUGIN_SOURCES
  gstchromiumsrc.cpp
  cef_gst_bridge.cpp
  cef_browser_client.cpp
  cef_manager.cpp
  gpu_utils.cpp
)

set(PLUGIN_HEADERS
  gstchromiumsrc.h
  cef_gst_bridge.h
  cef_browser_client.h
  cef_manager.h
  gpu_utils.h
  debug_utils.h
)

# GStreamer plugins are MODULE (shared library without SONAME)
add_library(gstchromiumsrc MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

target_include_directories(gstchromiumsrc PRIVATE
  ${CEF_INCLUDE_DIR}
  ${GST_INCLUDE_DIRS}
  ${GLIB_INCLUDE_DIRS}
)

target_compile_definitions(gstchromiumsrc PRIVATE
  ${COMMON_DEFINES}
)

target_compile_options(gstchromiumsrc PRIVATE
  ${GST_CFLAGS_OTHER}
  ${GLIB_CFLAGS_OTHER}
)

target_link_directories(gstchromiumsrc PRIVATE
  ${GST_LIBRARY_DIRS}
  ${GLIB_LIBRARY_DIRS}
)

# Platform-specific linking
if(OS_MAC)
  target_link_libraries(gstchromiumsrc PRIVATE
    ${CEF_WRAPPER_LIB}
    ${CEF_FRAMEWORK}
    ${GST_LIBRARIES}
    ${GLIB_LIBRARIES}
  )
  
  # Framework will be at @loader_path/Chromium Embedded Framework.framework/
  set_target_properties(gstchromiumsrc PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(OS_LINUX)
  target_link_libraries(gstchromiumsrc PRIVATE
    ${CEF_WRAPPER_LIB}
    ${CEF_LIB}
    ${GST_LIBRARIES}
    ${GLIB_LIBRARIES}
  )
  
  # Set RPATH for finding libcef.so relative to plugin
  set_target_properties(gstchromiumsrc PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

# GStreamer plugins use .so extension even on macOS
set_target_properties(gstchromiumsrc PROPERTIES
  PREFIX "lib"
  SUFFIX ".so"
)

# ============================================================================
# Subprocess Binary Target
# ============================================================================
set(SUBPROCESS_SOURCES
  subprocess_main.cpp
  gpu_utils.cpp
)

add_executable(chromiumsrc-subprocess ${SUBPROCESS_SOURCES} ${CMAKE_SOURCE_DIR}/gpu_utils.h)

target_include_directories(chromiumsrc-subprocess PRIVATE
  ${CEF_INCLUDE_DIR}
  ${GLIB_INCLUDE_DIRS}
)

target_compile_definitions(chromiumsrc-subprocess PRIVATE
  ${COMMON_DEFINES}
)

target_compile_options(chromiumsrc-subprocess PRIVATE
  ${GLIB_CFLAGS_OTHER}
)

target_link_directories(chromiumsrc-subprocess PRIVATE
  ${GLIB_LIBRARY_DIRS}
)

if(OS_MAC)
  target_link_libraries(chromiumsrc-subprocess PRIVATE
    ${CEF_WRAPPER_LIB}
    ${CEF_FRAMEWORK}
    ${GLIB_LIBRARIES}
  )
  
  # Framework will be at @executable_path/Chromium Embedded Framework.framework/
  set_target_properties(chromiumsrc-subprocess PROPERTIES
    INSTALL_RPATH "@executable_path"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
elseif(OS_LINUX)
  target_link_libraries(chromiumsrc-subprocess PRIVATE
    ${CEF_WRAPPER_LIB}
    ${CEF_LIB}
    ${GLIB_LIBRARIES}
  )
  
  set_target_properties(chromiumsrc-subprocess PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()

# ============================================================================
# Install Rules
# ============================================================================
if(NOT DEFINED ENV{HOME})
  message(FATAL_ERROR "HOME environment variable not set")
endif()

set(PLUGIN_DIR "$ENV{HOME}/.local/share/gstreamer-1.0/plugins")

install(TARGETS gstchromiumsrc chromiumsrc-subprocess
  LIBRARY DESTINATION ${PLUGIN_DIR}
  RUNTIME DESTINATION ${PLUGIN_DIR}
)

# Install CEF Framework and Resources to plugins directory
if(OS_MAC)
  # Install CEF framework alongside plugin (at @loader_path/)
  install(DIRECTORY "${CEF_FRAMEWORK_DIR}/"
    DESTINATION "${PLUGIN_DIR}/Chromium Embedded Framework.framework"
    PATTERN "libcef_dll_wrapper.a" EXCLUDE
    PATTERN "*.lib" EXCLUDE
  )
  
  # Fix framework install name to use @rpath for relative loading
  # CEF framework is built with @executable_path/../Frameworks/ which doesn't match our layout
  install(CODE "
    execute_process(
      COMMAND install_name_tool -id @rpath/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
              ${PLUGIN_DIR}/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
    )
    execute_process(
      COMMAND install_name_tool -change @executable_path/../Frameworks/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
              @rpath/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
              ${PLUGIN_DIR}/libgstchromiumsrc.so
    )
    execute_process(
      COMMAND install_name_tool -change @executable_path/../Frameworks/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
              @rpath/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
              ${PLUGIN_DIR}/chromiumsrc-subprocess
    )
    execute_process(
      COMMAND chmod +x ${PLUGIN_DIR}/Chromium\\ Embedded\\ Framework.framework/Chromium\\ Embedded\\ Framework
    )
  ")
elseif(OS_LINUX)
  install(DIRECTORY "${CEF_DIR}/Resources/"
    DESTINATION ${PLUGIN_DIR}
  )
  install(DIRECTORY "${CEF_DIR}/Release/"
    DESTINATION ${PLUGIN_DIR}
    PATTERN "libcef_dll_wrapper.a" EXCLUDE
    PATTERN "*.lib" EXCLUDE
  )
endif()

# ============================================================================
# Summary
# ============================================================================
message(STATUS "")
message(STATUS "=== gst-chromiumsrc Configuration ===")
message(STATUS "  Platform:      ${CMAKE_SYSTEM_NAME}")
message(STATUS "  CEF include:   ${CEF_INCLUDE_DIR}")
message(STATUS "  CEF wrapper:   ${CEF_WRAPPER_LIB}")
if(OS_MAC)
  message(STATUS "  CEF framework: ${CEF_FRAMEWORK}")
elseif(OS_LINUX)
  message(STATUS "  CEF library:   ${CEF_LIB}")
endif()
message(STATUS "  Plugin dir:    ${PLUGIN_DIR}")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "")
