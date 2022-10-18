#include "gstduktape.h"
#include "mod_dom.h"

static duk_ret_t native_print(duk_context *ctx);
static void push_file_as_string(duk_context *ctx, const char *filename);

int gst_duktape_init(duk_context **ctx)
{
    printf("[Duktape]:Init\n");

    *ctx = duk_create_heap_default();

    duk_push_global_object(*ctx);

    duk_push_c_function(*ctx, native_print, DUK_VARARGS);
    duk_put_prop_string(*ctx, -2, "print");

    // Load modules
    duk_push_c_function(*ctx, dukopen_dom_module, 0);
    duk_call(*ctx, 0);
    duk_put_global_string(*ctx, "svg");

    return 0;
}

int gst_duktape_load_script(duk_context *ctx, const char* location)
{
    printf("[Duktape]:Load script %s\n", location);

    push_file_as_string(ctx, location);
    if(duk_peval(ctx) != 0) {
        printf("Error running: %s\n", duk_safe_to_string(ctx, -1));
        return -1;
    }
    duk_pop(ctx);

    return 0;
}

int gst_duktape_start(duk_context *ctx, void *data)
{
    duk_get_prop_string(ctx, -1, "start");

    duk_get_global_string(ctx, "TrkSegment");
    duk_push_pointer(ctx, data);
    duk_new(ctx, 1);

    duk_int_t ret = duk_pcall(ctx, 1);
    if(ret != 0) {
        duk_safe_to_stacktrace(ctx, -1);
    } else {
        duk_safe_to_string(ctx, -1);
    }

    duk_pop(ctx);

    return 0;
}

int gst_duktape_render(duk_context *ctx, const char *buffer, size_t size, void *data)
{
    duk_get_prop_string(ctx, -1, "render");
    duk_push_string(ctx, (char *)data);
    
    duk_int_t ret = duk_pcall(ctx, 1);
    if(ret != 0) {
        duk_safe_to_stacktrace(ctx, -1);
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
    } 
    else {
        strcpy(buffer, duk_safe_to_string(ctx, -1));
    }
    duk_pop(ctx);

    return strlen(buffer);
}

duk_ret_t native_print(duk_context *ctx) 
{
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    // duk_safe_to_string(ctx, -1);
    printf("  [JavaScript] %s\n", duk_safe_to_string(ctx, -1));
    return 0;
}

void push_file_as_string(duk_context *ctx, const char *filename) {
    FILE *f;
    size_t len;
    char buf[16384];

    f = fopen(filename, "rb");
    if (f) {
        len = fread((void *) buf, 1, sizeof(buf), f);
        fclose(f);
        duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);
    } else {
        duk_push_undefined(ctx);
    }
}