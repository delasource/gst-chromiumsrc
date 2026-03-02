#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

G_BEGIN_DECLS

static gchar _debug_instance_id[7] = {0};
static gboolean _debug_initialized = FALSE;

/**
 * debug_init:
 *
 * Initializes the debug system with a random 6-character identifier.
 * Should be called once at process start.
 */
static inline void debug_init(void) {
    if (_debug_initialized) return;
    
    srand(time(NULL) ^ getpid());
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < 6; i++) {
        _debug_instance_id[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    _debug_instance_id[6] = '\0';
    _debug_initialized = TRUE;
}

/**
 * debug_get_id:
 *
 * Returns the 6-character instance identifier.
 * Initializes if not already done.
 */
static inline const gchar* debug_get_id(void) {
    if (!_debug_initialized) {
        debug_init();
    }
    return _debug_instance_id;
}

/**
 * DEBUG_LOG macro:
 *
 * Logs a debug message with the instance ID prefix.
 * Usage: DEBUG_LOG("message with %s", arg);
 */
#define DEBUG_LOG(fmt, ...) \
    g_print("[%s] " fmt "\n", debug_get_id(), ##__VA_ARGS__)

/**
 * DEBUG_LOG_GL macro:
 *
 * Logs a GL-specific debug message with instance ID and GL prefix.
 */
#define DEBUG_LOG_GL(fmt, ...) \
    g_print("[%s] GL: " fmt "\n", debug_get_id(), ##__VA_ARGS__)

/**
 * DEBUG_LOG_CEF macro:
 *
 * Logs a CEF-specific debug message with instance ID and CEF prefix.
 */
#define DEBUG_LOG_CEF(fmt, ...) \
    g_print("[%s] CEF: " fmt "\n", debug_get_id(), ##__VA_ARGS__)

/**
 * DEBUG_LOG_GST macro:
 *
 * Logs a GStreamer-specific debug message with instance ID and GST prefix.
 */
#define DEBUG_LOG_GST(fmt, ...) \
    g_print("[%s] GST: " fmt "\n", debug_get_id(), ##__VA_ARGS__)

/**
 * debug_check_gl_error:
 * @context: Description of what was being done when checking
 *
 * Checks for OpenGL errors and logs them if found.
 * Returns TRUE if no error, FALSE if error detected.
 */
static inline gboolean debug_check_gl_error(const gchar *context) {
#ifdef HAVE_GL
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        const gchar *err_str = "UNKNOWN";
        switch (err) {
            case GL_INVALID_ENUM: err_str = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: err_str = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: err_str = "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: err_str = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: err_str = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        DEBUG_LOG_GL("Error in %s: %s (0x%x)", context, err_str, err);
        return FALSE;
    }
    return TRUE;
#else
    return TRUE;
#endif
}

/**
 * debug_log_gl_info:
 *
 * Logs OpenGL/EGL information if available.
 */
static inline void debug_log_gl_info(void) {
    DEBUG_LOG_GL("=== GL Information ===");
    
    const gchar *display_env = g_getenv("DISPLAY");
    const gchar *wayland_display = g_getenv("WAYLAND_DISPLAY");
    DEBUG_LOG_GL("DISPLAY env: %s", display_env ? display_env : "(not set)");
    DEBUG_LOG_GL("WAYLAND_DISPLAY env: %s", wayland_display ? wayland_display : "(not set)");
    
#ifdef HAVE_GL
    const gchar *vendor = (const gchar*)glGetString(GL_VENDOR);
    const gchar *renderer = (const gchar*)glGetString(GL_RENDERER);
    const gchar *version = (const gchar*)glGetString(GL_VERSION);
    const gchar *glsl_version = (const gchar*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    
    DEBUG_LOG_GL("Vendor: %s", vendor ? vendor : "(null)");
    DEBUG_LOG_GL("Renderer: %s", renderer ? renderer : "(null)");
    DEBUG_LOG_GL("Version: %s", version ? version : "(null)");
    DEBUG_LOG_GL("GLSL: %s", glsl_version ? glsl_version : "(null)");
#else
    DEBUG_LOG_GL("GL not linked - no runtime info available");
#endif
    
    DEBUG_LOG_GL("======================");
}

G_END_DECLS

#endif
