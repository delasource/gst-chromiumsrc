#ifndef __GPU_UTILS_H__
#define __GPU_UTILS_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct {
    gboolean enabled;
    gint device_index;
    gchar *device_path;
} GpuConfig;

gint gpu_detect_best_device(void);
gboolean gpu_is_available(void);
GpuConfig *gpu_config_new(void);
void gpu_config_free(GpuConfig *config);
void gpu_config_detect(GpuConfig *config);

G_END_DECLS

#endif
