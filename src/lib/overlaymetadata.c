#include "overlaymetadata.h"

GType
overlay_meta_api_get_type (void)
{
    static volatile GType type;
    static const gchar *tags[] = { "foo", "bar", NULL };
    
    if (g_once_init_enter (&type)) {
        GType _type = gst_meta_api_type_register("OverlayMetaAPI", tags);
        g_once_init_leave(&type, _type);
    }
    return type;
}

static gboolean
overlay_meta_init(GstMeta * meta, gpointer params, GstBuffer * buffer)
{
    OverlayMeta *emeta = (OverlayMeta *) meta;
    emeta->data = NULL;

    return TRUE;
}

static void 
overlay_meta_free(GstMeta *meta, GstBuffer *buffer)
{
    OverlayMeta *emeta = (OverlayMeta *)meta;
    if(meta != NULL && emeta->data != NULL) {
        g_free(emeta->data);
    }
}

static gboolean
overlay_meta_transform(GstBuffer * transbuf, GstMeta * meta,
                                GstBuffer * buffer, GQuark type, gpointer data)
{
    OverlayMeta *emeta = (OverlayMeta *)meta;
    gst_buffer_add_overlay_meta(transbuf, g_strdup(emeta->data));
    return TRUE;
}

const GstMetaInfo *
overlay_meta_get_info (void)
{
    static const GstMetaInfo *meta_info = NULL;
    
    if (g_once_init_enter (&meta_info)) {
        const GstMetaInfo *mi = gst_meta_register ( OVERLAY_META_API_TYPE,
                                                    "OverlayMeta",
                                                    sizeof(OverlayMeta),
                                                    overlay_meta_init,
                                                    overlay_meta_free,
                                                    overlay_meta_transform);
                                                    g_once_init_leave(&meta_info, mi);
    }

    return meta_info;
}

OverlayMeta *
gst_buffer_add_overlay_meta(GstBuffer *buffer, gchar *data)
{
    OverlayMeta *meta = NULL;
    
    g_return_val_if_fail (GST_IS_BUFFER (buffer), NULL);
    meta = (OverlayMeta *) gst_buffer_add_meta(buffer, OVERLAY_META_INFO, NULL);
    if(G_UNLIKELY(meta == NULL)) {
        return NULL;
    }

    meta->data = data;
    
    return meta;
}