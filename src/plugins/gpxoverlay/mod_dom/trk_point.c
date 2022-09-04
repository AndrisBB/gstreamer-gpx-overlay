#include "trk_point.h"
#include "gpxparser.h"

typedef struct
{
    gpx_trk_point_t *priv;
} 
trk_point_t;

static trk_point_t* _trk_point_stack_get_ptr(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "TrkPoint");

    trk_point_t *point = (trk_point_t *)duk_to_pointer(ctx, -1);

    duk_pop(ctx);

    return point;
}

static duk_ret_t trk_point_idx_getter(duk_context *ctx)
{
    trk_point_t *point = _trk_point_stack_get_ptr(ctx);
    if(point == NULL) {
        printf("Trk_point is NULL\n");
        return 0;
    }

    if(point->priv == NULL) {
        duk_push_int(ctx, -1);
    }
    else {
        duk_push_int(ctx, point->priv->idx);
    }
    
    return 1;
}

static duk_ret_t trk_point_timestamp_setter(duk_context *ctx)
{
    const char *timestamp = duk_safe_to_string(ctx, -1);
    printf("Set id to:%s, do nothing\n", timestamp);

    return 0;
}

static duk_ret_t trk_point_timestamp_getter(duk_context *ctx)
{
    trk_point_t *point = _trk_point_stack_get_ptr(ctx);
    if(point == NULL) {
        printf("Trk_point is NULL\n");
        return 0;
    }

    if(point->priv == NULL) {
        duk_push_string(ctx, "");
    }
    else {
        duk_push_string(ctx, g_date_time_format_iso8601(point->priv->timestamp));
    }
    
    return 1;
}

static duk_ret_t trk_point_hr_setter(duk_context *ctx)
{
    gint hr = duk_to_int(ctx, -1);
    printf("Set id to:%d, do nothing\n", hr);

    return 0;
}

static duk_ret_t trk_point_hr_getter(duk_context *ctx)
{
    trk_point_t *point = _trk_point_stack_get_ptr(ctx);
    if(point == NULL) {
        printf("Trk_point is NULL\n");
        return 0;
    }

    if(point->priv == NULL) {
        duk_push_int(ctx, 8976);
    }
    else {
        duk_push_int(ctx, point->priv->hr);
    }
    
    return 1;
}

static duk_ret_t trk_point_elevation_getter(duk_context *ctx)
{
    trk_point_t *point = _trk_point_stack_get_ptr(ctx);
    if(point == NULL) {
        printf("Trk_point is NULL\n");
        return 0;
    }

    if(point->priv == NULL) {
        duk_push_number(ctx, 0);
    }
    else {
        duk_push_number(ctx, point->priv->elevation);
    }
    
    return 1;
}

static duk_ret_t trk_point_elevation_setter(duk_context *ctx)
{
    gfloat elevation = (gfloat)duk_to_number(ctx, -1);
    printf("Set elevation to:%f, do nothing\n", elevation);

    return 0;
}

static duk_ret_t trk_point_destructor(duk_context *ctx)
{
    trk_point_t *point = _trk_point_stack_get_ptr(ctx);
    if(point == NULL) {
        printf("TrkPoint is NULL\n");
        return 0;
    }

    free(point);

    return 0;
}

static duk_ret_t trk_point_constructor(duk_context *ctx) 
{
    if(!duk_is_constructor_call(ctx)) {
        printf("This is not a constructor call\n");
        return DUK_RET_TYPE_ERROR;
    }

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_TYPE_ERROR;
    }

    gpx_trk_point_t *priv = (gpx_trk_point_t *)duk_to_pointer(ctx, 0);
    if(priv == NULL) {
        printf("No trk_point pointer given\n");
    }

    trk_point_t *point = malloc(sizeof(trk_point_t));
    point->priv = priv;

    duk_push_this(ctx);
    
    duk_push_string(ctx, "idx");
    duk_push_c_function(ctx, trk_point_idx_getter, 0);
    duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_GETTER);

    duk_push_string(ctx, "timestamp");
    duk_push_c_function(ctx, trk_point_timestamp_getter, 0);
    duk_push_c_function(ctx, trk_point_timestamp_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
    
    duk_push_string(ctx, "hr");
    duk_push_c_function(ctx, trk_point_hr_getter, 0);
    duk_push_c_function(ctx, trk_point_hr_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_string(ctx, "elevation");
    duk_push_c_function(ctx, trk_point_elevation_getter, 0);
    duk_push_c_function(ctx, trk_point_elevation_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_pointer(ctx, (void *)point);
    duk_put_prop_string(ctx, -2, "TrkPoint");

    duk_push_c_function(ctx, trk_point_destructor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
} 

duk_ret_t trk_point_init(duk_context *ctx)
{
    duk_push_c_function(ctx, trk_point_constructor, 1);
    duk_push_object(ctx);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "TrkPoint");
    
    return 0;
}