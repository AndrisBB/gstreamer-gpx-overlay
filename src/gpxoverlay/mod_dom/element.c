#include "element.h"

#include <gdome.h>

typedef struct
{
    GdomeElement *el;
} 
element_t;

static element_t* _element_stack_get_element_ptr(duk_context *ctx)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "Element");

    element_t *element = (element_t *)duk_to_pointer(ctx, -1);

    duk_pop(ctx);

    return element;
}

static duk_ret_t element_id_setter(duk_context *ctx)
{
    const char *id = duk_safe_to_string(ctx, -1);
    printf("Set id to:%s\n", id);

    return 0;
}

static duk_ret_t element_id_getter(duk_context *ctx)
{
    int ret = 0;

    element_t *element = _element_stack_get_element_ptr(ctx);
    if(element == NULL) {
        printf("Element is NULL\n");
        return 0;
    }

    GdomeException exc;
    GdomeDOMString *id_attr = gdome_str_mkref("id");
    GdomeDOMString *id_value = gdome_el_getAttribute(element->el, id_attr, &exc);
    
    if(id_value != NULL) {
        duk_push_string(ctx, id_value->str);
        ret = 1;
    }

    gdome_str_unref(id_attr);
    gdome_str_unref(id_value);

    return ret;
}

static duk_ret_t element_inner_html_setter(duk_context *ctx)
{
    GdomeException exc;

    const char *html = duk_safe_to_string(ctx, -1);

    element_t *element = _element_stack_get_element_ptr(ctx);
    if(element == NULL) {
        printf("Element is NULL\n");
        return 0;
    }

    // Check if element has children and remove them
    if(gdome_el_hasChildNodes(element->el, &exc)) {
        GdomeNodeList *childs = gdome_el_childNodes(element->el, &exc);
        if(childs != NULL) {
            unsigned long nchilds = gdome_nl_length(childs, &exc);
            for(unsigned long i = 0; i < nchilds; i++) {
                GdomeNode *node = gdome_nl_item(childs, i, &exc);
                GdomeNode *removed_node = gdome_el_removeChild(element->el, node, &exc);

                gdome_n_unref(node, &exc);
                gdome_n_unref(removed_node, &exc);
            }

            gdome_nl_unref(childs, &exc);
        }
    }
    
    // Create new node for the innerHTML
    GdomeDOMString *value = gdome_str_mkref(html);
    GdomeText *txtnode = gdome_doc_createTextNode(gdome_el_ownerDocument(element->el, &exc), value, &exc);
    if(txtnode == NULL) {
        printf("Failed to create text node\n");
        return 0;
    }
    gdome_str_unref(value);

    GdomeNode *result = gdome_el_appendChild(element->el, (GdomeNode *)txtnode, &exc);
    if(result != (GdomeNode *)txtnode) {
        printf("Failed to append child node\n");
        return 0;
    }
    gdome_t_unref(txtnode, &exc);
    gdome_n_unref(result, &exc);

    return 0;
}

static duk_ret_t element_inner_html_getter(duk_context *ctx)
{
    int ret = 0;
    GdomeException exc;

    printf("element_inner_html_getter called\n");

    element_t *element = _element_stack_get_element_ptr(ctx);
    if(element == NULL) {
        printf("Element is NULL\n");
        return 0;
    }

    printf("Element type is:%u\n", gdome_el_nodeType(element->el, &exc));
    printf("loop child nodes\n");

    GdomeNodeList *childs = gdome_el_childNodes(element->el, &exc);
    if(childs != NULL) {
        printf("Children are not NULL\n");
        
        unsigned long nchilds = gdome_nl_length(childs, &exc);
        for(unsigned long i = 0; i < nchilds; i++) {
            GdomeElement *el = (GdomeElement *)gdome_nl_item(childs, i, &exc);
            printf("Element [%d] type is:%u\n", (int)i, gdome_el_nodeType(el, &exc));

            if(gdome_el_nodeType(el, &exc) == 3) {
                GdomeDOMString *value = gdome_el_nodeValue(el, &exc);
                if(value != NULL) {
                    printf("value:%s\n", value->str);
                }
            }

        }
    }

    return ret;
}

static duk_ret_t element_set_attribute(duk_context *ctx)
{
    GdomeException exc;

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_ERROR;
    }

    const char *value = duk_safe_to_string(ctx, -1);
    const char *name = duk_safe_to_string(ctx, -2);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "Element");
    element_t *element = (element_t *)duk_to_pointer(ctx, -1);

    if(element == NULL) {
        printf("element is NULL\n");
        return 0;
    }
    duk_pop(ctx);

    GdomeDOMString *name_str = gdome_str_mkref(name);
    GdomeDOMString *value_str = gdome_str_mkref(value);

    gdome_el_setAttribute(element->el, name_str, value_str, &exc);

    gdome_str_unref(name_str);
    gdome_str_unref(value_str);

    return 0;
}

static duk_ret_t element_destructor(duk_context *ctx)
{
    element_t *element = _element_stack_get_element_ptr(ctx);
    if(element == NULL) {
        printf("Element is NULL\n");
        return 0;
    }

    GdomeException exc;
    gdome_el_unref(element->el, &exc);

    free(element);

    return 0;
}

static duk_ret_t element_constructor(duk_context *ctx) 
{
    if(!duk_is_constructor_call(ctx)) {
        printf("This is not a constructor call\n");
        return DUK_RET_TYPE_ERROR;
    }

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_TYPE_ERROR;
    }

    GdomeElement *el = duk_to_pointer(ctx, 0);
    if(el == NULL) {
        printf("No element pointer given\n");
    }

    element_t *element = malloc(sizeof(element_t));
    element->el = el;

    duk_push_this(ctx);
    
    duk_push_string(ctx, "id");
    duk_push_c_function(ctx, element_id_getter, 0);
    duk_push_c_function(ctx, element_id_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
    
    duk_push_string(ctx, "innerHTML");
    duk_push_c_function(ctx, element_inner_html_getter, 0);
    duk_push_c_function(ctx, element_inner_html_setter, 1);
    duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    // duk_push_string(ctx, "innerHTML");
    // duk_push_c_function(ctx, element_inner_html_getter, 0);
    // duk_push_c_function(ctx, element_inner_html_setter, 1);
    // duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);

    duk_push_pointer(ctx, (void *)element);
    duk_put_prop_string(ctx, -2, "Element");

    duk_push_c_function(ctx, element_destructor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
} 

static duk_function_list_entry element_funcs[] = {
    { "setAttribute",     element_set_attribute,      2 },
    { NULL, NULL, 0 }
};

duk_ret_t element_init(duk_context *ctx)
{
    duk_push_c_function(ctx, element_constructor, 1);
    duk_push_object(ctx);

    duk_put_function_list(ctx, -1, element_funcs);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "Element");
    
    return 0;
}