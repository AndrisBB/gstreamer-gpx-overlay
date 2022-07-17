#include "document.h"

#include <gdome.h>

typedef struct 
{
    GdomeDOMImplementation *domimpl;
    GdomeDocument *doc;
} document_t;

static duk_ret_t document_get_element_by_id(duk_context *ctx)
{
    GdomeException exc;

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_ERROR;
    }

    const char *id = duk_safe_to_string(ctx, -1);

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "Document");
    document_t *document = (document_t *)duk_to_pointer(ctx, -1);
    if(document == NULL) {
        printf("document is NULL\n");
        return 0;
    }
    duk_pop(ctx);
    
    GdomeDOMString *dom_id = gdome_str_mkref(id);
    GdomeElement *element = gdome_doc_getElementById(document->doc, dom_id, &exc);
    gdome_str_unref(dom_id);

    if(element == NULL) {
        printf("No element found\n");
        return 0;
    }
    
    duk_get_global_string(ctx, "Element");
    duk_push_pointer(ctx, element);
    duk_new(ctx, 1);

    return 1;
}

static duk_ret_t document_stringify(duk_context *ctx) 
{
    // printf("Stringify\n");

    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "Document");
    document_t *document = (document_t *)duk_to_pointer(ctx, -1);
    if(document == NULL) {
        printf("document is NULL\n");
        return 0;
    }
    duk_pop(ctx);

    GdomeException exc;
    char *data = NULL;
    GdomeBoolean success = gdome_di_saveDocToMemory(document->domimpl,
                                                    document->doc,
                                                    &data,
                                                    GDOME_SAVE_STANDARD,
                                                    &exc);
    if(!success) {
        return DUK_ERR_ERROR;
    }

    duk_push_string(ctx, (const char *)data);
    free(data);

    return 1;
}

static duk_ret_t document_destructor(duk_context *ctx)
{
    printf("Destructor called, free document\n");
    return 0;
}

static duk_ret_t document_constructor(duk_context *ctx) 
{
    printf("Constructor called\n");

    if(!duk_is_constructor_call(ctx)) {
        printf("This is not a constructor call\n");
        return DUK_RET_TYPE_ERROR;
    }

    if(duk_get_top(ctx) == 0) {
        printf("No argument given\n");
        return DUK_RET_TYPE_ERROR;
    }

    const char *location = duk_to_string(ctx, 0);
    if(location != NULL) {
        printf("Parse SVG file:%s\n", location);
    }

    GdomeDOMImplementation *domimpl = gdome_di_mkref();
    GdomeException exc;
    GdomeDocument *doc = gdome_di_createDocFromURI(domimpl, location, GDOME_LOAD_PARSING, &exc);
    if(doc == NULL) {
        printf("DOMImplementation.createDocFromURI: failed\n\tException #%d\n", exc);
        return DUK_ERR_URI_ERROR;
    }

    document_t *document = malloc(sizeof(document_t));
    document->domimpl = domimpl;
    document->doc = doc;

    duk_push_this(ctx);
    duk_push_pointer(ctx, (void *)document);
    duk_put_prop_string(ctx, -2, "Document");

    duk_push_c_function(ctx, document_destructor, 1);
    duk_set_finalizer(ctx, -2);

    return 0;
} 

static duk_function_list_entry document_funcs[] = {
    { "stringify",          document_stringify,              0 },
    { "getElementById",     document_get_element_by_id,      1 },
    { NULL, NULL, 0 }
};

duk_ret_t document_init(duk_context *ctx)
{
    duk_push_c_function(ctx, document_constructor, 1);
    duk_push_object(ctx);

    duk_put_function_list(ctx, -1, document_funcs);

    duk_put_prop_string(ctx, -2, "prototype");
    duk_put_global_string(ctx, "Document");

    return 0;
}