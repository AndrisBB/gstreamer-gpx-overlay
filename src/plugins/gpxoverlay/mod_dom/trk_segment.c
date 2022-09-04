#include "trk_segment.h"
#include "duk_utils.h"
#include "gpxparser.h"

typedef struct
{
    gpx_trk_segment_t *priv;
} 
trk_segment_t;

static duk_ret_t element_trk_points_getter(duk_context *ctx)
{
    trk_segment_t *segment = (trk_segment_t *)duk_utils_get_stack_pointer(ctx, "TrkSegment");
    if(segment == NULL) {
        printf("TrkSegment is NULL\n");
        return 0;
    }

    duk_idx_t arr_idx;

    arr_idx = duk_push_array(ctx);

    GList *elem;
    gint idx = 0;
    for(elem = segment->priv->trk_points; elem; elem = elem->next) {
        gpx_trk_point_t *point = (gpx_trk_point_t *)elem->data;
        duk_get_global_string(ctx, "TrkPoint");
        duk_push_pointer(ctx, point);
        duk_new(ctx, 1);
        duk_put_prop_index(ctx, arr_idx, idx++);
    }

    return 1;
}

static duk_ret_t trk_segment_destructor(duk_context *ctx)
{
    trk_segment_t *segment = (trk_segment_t *)duk_utils_get_stack_pointer(ctx, "TrkSegment");
    if(segment == NULL) {
        printf("TrkSegment is NULL\n");
        return 0;
    }

    free(segment);

    return 0;
}

static duk_ret_t trk_segment_constructor(duk_context *ctx) 
{
    if(!duk_is_constructor_call(ctx)) {
        printf("This is not a constructor call\n");
        return DUK_RET_TYPE_ERROR;
    }

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_TYPE_ERROR;
    }

    gpx_trk_segment_t *priv = (gpx_trk_segment_t *)duk_to_pointer(ctx, 0);
    if(priv == NULL) {
        printf("No trk_segment pointer given\n");
    }

    trk_segment_t *segment = malloc(sizeof(trk_segment_t));
    segment->priv = priv;

    duk_push_this(ctx);
    
    duk_push_string(ctx, "trkPoints");
    duk_push_c_function(ctx, element_trk_points_getter, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);
    duk_dump_object(ctx, -1);

    duk_push_pointer(ctx, (void *)segment);
    duk_put_prop_string(ctx, -2, "TrkSegment");

    duk_push_c_function(ctx, trk_segment_destructor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
} 

duk_ret_t trk_segment_init(duk_context *ctx)
{
    duk_push_c_function(ctx, trk_segment_constructor, 1);
    duk_push_object(ctx);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "TrkSegment");
    
    return 0;
}