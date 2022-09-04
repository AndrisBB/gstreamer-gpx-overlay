#include <gst/gst.h>

typedef struct _OverlayMeta OverlayMeta;

struct _OverlayMeta {
    GstMeta         meta;
    gchar *         data;
};

GType overlay_meta_api_get_type (void);
#define OVERLAY_META_API_TYPE (overlay_meta_api_get_type())

#define gst_buffer_get_overlay_meta(b) ((OverlayMeta*) gst_buffer_get_meta((b),OVERLAY_META_API_TYPE))

const GstMetaInfo* overlay_meta_get_info (void);
#define OVERLAY_META_INFO (overlay_meta_get_info())

OverlayMeta* gst_buffer_add_overlay_meta(GstBuffer *buffer, gchar *data);