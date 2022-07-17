#include "trk_point.h"
#include "gpxparser.h"

typedef struct
{
    gpx_trk_point *priv;
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
        duk_push_int(ctx, 0);
    }
    else {
        duk_push_int(ctx, point->priv->hr);
    }
    
    return 1;
}

// static duk_ret_t element_inner_html_setter(duk_context *ctx)
// {
//     GdomeException exc;

//     const char *html = duk_safe_to_string(ctx, -1);

//     element_t *element = _element_stack_get_element_ptr(ctx);
//     if(element == NULL) {
//         printf("Element is NULL\n");
//         return 0;
//     }

//     // Check if element has children and remove them
//     if(gdome_el_hasChildNodes(element->el, &exc)) {
//         GdomeNodeList *childs = gdome_el_childNodes(element->el, &exc);
//         if(childs != NULL) {
//             unsigned long nchilds = gdome_nl_length(childs, &exc);
//             for(unsigned long i = 0; i < nchilds; i++) {
//                 GdomeNode *node = gdome_nl_item(childs, i, &exc);
//                 GdomeNode *removed_node = gdome_el_removeChild(element->el, node, &exc);

//                 gdome_n_unref(node, &exc);
//                 gdome_n_unref(removed_node, &exc);
//             }

//             gdome_nl_unref(childs, &exc);
//         }
//     }
    
//     // Create new node for the innerHTML
//     GdomeDOMString *value = gdome_str_mkref(html);
//     GdomeText *txtnode = gdome_doc_createTextNode(gdome_el_ownerDocument(element->el, &exc), value, &exc);
//     if(txtnode == NULL) {
//         printf("Failed to create text node\n");
//         return 0;
//     }
//     gdome_str_unref(value);

//     GdomeNode *result = gdome_el_appendChild(element->el, (GdomeNode *)txtnode, &exc);
//     if(result != (GdomeNode *)txtnode) {
//         printf("Failed to append child node\n");
//         return 0;
//     }
//     gdome_t_unref(txtnode, &exc);
//     gdome_n_unref(result, &exc);

//     return 0;
// }

// static duk_ret_t element_inner_html_getter(duk_context *ctx)
// {
//     int ret = 0;
//     GdomeException exc;

//     printf("element_inner_html_getter called\n");

//     element_t *element = _element_stack_get_element_ptr(ctx);
//     if(element == NULL) {
//         printf("Element is NULL\n");
//         return 0;
//     }

//     printf("Element type is:%u\n", gdome_el_nodeType(element->el, &exc));
//     printf("loop child nodes\n");

//     GdomeNodeList *childs = gdome_el_childNodes(element->el, &exc);
//     if(childs != NULL) {
//         printf("Children are not NULL\n");
        
//         unsigned long nchilds = gdome_nl_length(childs, &exc);
//         for(unsigned long i = 0; i < nchilds; i++) {
//             GdomeElement *el = (GdomeElement *)gdome_nl_item(childs, i, &exc);
//             printf("Element [%d] type is:%u\n", (int)i, gdome_el_nodeType(el, &exc));

//             if(gdome_el_nodeType(el, &exc) == 3) {
//                 GdomeDOMString *value = gdome_el_nodeValue(el, &exc);
//                 if(value != NULL) {
//                     printf("value:%s\n", value->str);
//                 }
//             }

//         }
//     }

//     return ret;
// }

static duk_ret_t trk_point_destructor(duk_context *ctx)
{
    // printf("TrkPoint destructor\n");

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
    // printf("TrkPoint constructor\n");

    if(!duk_is_constructor_call(ctx)) {
        printf("This is not a constructor call\n");
        return DUK_RET_TYPE_ERROR;
    }

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_TYPE_ERROR;
    }

    gpx_trk_point *priv = (gpx_trk_point *)duk_to_pointer(ctx, 0);
    if(priv == NULL) {
        printf("No trk_point pointer given\n");
    }

    trk_point_t *point = malloc(sizeof(trk_point_t));
    point->priv = priv;

    duk_push_this(ctx);
    
    duk_push_string(ctx, "timestamp");
    duk_push_c_function(ctx, trk_point_timestamp_getter, 0);
    duk_push_c_function(ctx, trk_point_timestamp_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
    
    duk_push_string(ctx, "hr");
    duk_push_c_function(ctx, trk_point_hr_getter, 0);
    duk_push_c_function(ctx, trk_point_hr_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_pointer(ctx, (void *)point);
    duk_put_prop_string(ctx, -2, "TrkPoint");

    duk_push_c_function(ctx, trk_point_destructor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
} 

duk_ret_t trk_point_init(duk_context *ctx)
{
    printf("TrkPoint init\n");

    duk_push_c_function(ctx, trk_point_constructor, 1);
    duk_push_object(ctx);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "TrkPoint");
    
    return 0;
}