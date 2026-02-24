#include "gpu_utils.h"
#include <glib.h>

gint gpu_detect_best_device(void) {
    GDir *dir = g_dir_open("/dev/dri", 0, NULL);
    if (!dir) return -1;

    gint highest_render = -1;
    const gchar *name;
    while ((name = g_dir_read_name(dir)) != NULL) {
        if (g_str_has_prefix(name, "renderD")) {
            gint num = atoi(name + 7);
            if (num > highest_render) {
                highest_render = num;
            }
        }
    }
    g_dir_close(dir);
    return highest_render;
}

gboolean gpu_is_available(void) {
    return g_file_test("/dev/dri", G_FILE_TEST_IS_DIR) &&
           gpu_detect_best_device() >= 0;
}

GpuConfig *gpu_config_new(void) {
    GpuConfig *config = g_new0(GpuConfig, 1);
    config->enabled = FALSE;
    config->device_index = -1;
    config->device_path = NULL;
    return config;
}

void gpu_config_free(GpuConfig *config) {
    if (config) {
        g_free(config->device_path);
        g_free(config);
    }
}

void gpu_config_detect(GpuConfig *config) {
    if (!config) return;

    config->device_index = gpu_detect_best_device();
    if (config->device_index >= 0) {
        config->enabled = TRUE;
        config->device_path = g_strdup_printf("/dev/dri/renderD%d", config->device_index);
    } else {
        config->enabled = FALSE;
        config->device_path = NULL;
    }
}
