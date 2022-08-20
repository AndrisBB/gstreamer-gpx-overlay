#include "document.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef struct 
{
    xmlDocPtr xml_doc;
} document_t;

static xmlNode* find_node(xmlNode * node, char * prop_val) 
{
    xmlNode *result;

    if(node == NULL) return NULL;

    while(node) {
        if((node->type == XML_ELEMENT_NODE) && xmlGetProp(node, "id") && (strcmp(xmlGetProp(node, "id"), prop_val) == 0)) {
            return node;
        }
    
        if(result = find_node(node->children, prop_val)) return result;
    
        node = node->next;
    }
  
    return NULL;
}

static duk_ret_t document_get_element_by_id(duk_context *ctx)
{
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
        
    xmlNode *root = xmlDocGetRootElement(document->xml_doc);
    if(root == NULL) {
        printf("[Document] root element is null\n");
        return 0;
    }

    xmlNode *node = find_node(root, id);
    if(!node) {
        printf("[Document] Failed to find node\n");
        return 0;
    }

    duk_get_global_string(ctx, "Element");
    duk_push_pointer(ctx, node);
    duk_new(ctx, 1);

    return 1;
}

static duk_ret_t document_stringify(duk_context *ctx) 
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, "Document");
    document_t *document = (document_t *)duk_to_pointer(ctx, -1);
    if(document == NULL) {
        printf("document is NULL\n");
        return 0;
    }
    duk_pop(ctx);

    xmlChar *xmlbuff = NULL;
    int buffersize = 0;

    xmlDocDumpMemory(document->xml_doc, &xmlbuff, &buffersize);

    duk_push_string(ctx, (const char *)xmlbuff);
    xmlFree(xmlbuff);

    return 1;
}

static duk_ret_t document_destructor(duk_context *ctx)
{
    // TODO: Clean up document
    // xmlFreeDoc(doc);

    return 0;
}

static duk_ret_t document_constructor(duk_context *ctx) 
{
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

    document_t *document = malloc(sizeof(document_t));

    // Load XML document
    document->xml_doc = xmlReadFile(location, NULL, 0);
    if(document->xml_doc == NULL) {
        printf("[Document] Failed to parse %s\n", location);
        return DUK_RET_ERROR;
    }

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