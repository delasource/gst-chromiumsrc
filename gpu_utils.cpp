#include "gpu_utils.h"
#include "debug_utils.h"
#include <glib.h>

/**
 * gpu_detect_best_device:
 *
 * Scans the /dev/dri directory for available GPU render devices.
 * Returns the highest numbered renderD device (typically the most capable GPU).
 *
 * Returns: The device number (e.g., 128 for renderD128), or -1 if none found
 */
gint gpu_detect_best_device(void) {
    GDir *dir = g_dir_open("/dev/dri", 0, NULL);
    if (!dir) {
        DEBUG_LOG_GL("detect_best_device - Cannot open /dev/dri directory");
        return -1;
    }

    gint highest_render = -1;
    gint device_count = 0;
    const gchar *name;
    
    while ((name = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_prefix(name, "renderD")) {
            gint num = atoi(name + 7);
            device_count++;
            DEBUG_LOG_GL("detect_best_device - Found device: %s (number: %d)", name, num);
            if (num > highest_render) {
                highest_render = num;
            }
        }
    }
    g_dir_close(dir);
    
    DEBUG_LOG_GL("detect_best_device - Total devices found: %d, best device: %d", 
            device_count, highest_render);
    return highest_render;
}

/**
 * gpu_is_available:
 *
 * Checks if any GPU devices are available on the system.
 *
 * Returns: TRUE if /dev/dri exists and contains at least one render device
 */
gboolean gpu_is_available(void) {
    gboolean dir_exists = g_file_test("/dev/dri", G_FILE_TEST_IS_DIR);
    gint device = gpu_detect_best_device();
    gboolean available = dir_exists && device >= 0;
    return available;
}

/**
 * gpu_config_new:
 *
 * Creates a new GpuConfig structure with default values (disabled, no device).
 *
 * Returns: A newly allocated GpuConfig, should be freed with gpu_config_free()
 */
GpuConfig *gpu_config_new(void) {
    GpuConfig *config = g_new0(GpuConfig, 1);
    config->enabled = FALSE;
    config->device_index = -1;
    config->device_path = NULL;
    return config;
}

/**
 * gpu_config_free:
 * @config: The GpuConfig to free
 *
 * Frees a GpuConfig structure and its associated resources.
 */
void gpu_config_free(GpuConfig *config) {
    if (config) {
        g_free(config->device_path);
        g_free(config);
    }
}

/**
 * gpu_config_detect:
 * @config: The GpuConfig to populate
 *
 * Detects the best available GPU device and populates the config structure.
 * Sets enabled=TRUE if a device is found, along with its index and path.
 */
void gpu_config_detect(GpuConfig *config) {
    if (!config) {
        DEBUG_LOG_GL("config_detect - Config is NULL, skipping");
        return;
    }

    config->device_index = gpu_detect_best_device();
    if (config->device_index >= 0) {
        config->enabled = TRUE;
        config->device_path = g_strdup_printf("/dev/dri/renderD%d", config->device_index);
        DEBUG_LOG_GL("config_detect - GPU detected and enabled: %s", config->device_path);
        DEBUG_LOG_GL("config_detect - Device index: %d", config->device_index);
        
        // Check if device file is accessible
        if (g_file_test(config->device_path, G_FILE_TEST_EXISTS)) {
        } else {
            DEBUG_LOG_GL("config_detect - WARNING: Device file does not exist!");
        }
    } else {
        config->enabled = FALSE;
        config->device_path = NULL;
        DEBUG_LOG_GL("config_detect - No GPU device found, GPU disabled");
    }
}
