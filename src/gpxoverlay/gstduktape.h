#ifndef GST_DUKTAPE_H
#define GST_DUKTAPE_H

#include "duktape.h"

int gst_duktape_init(duk_context **ctx);
int gst_duktape_load_script(duk_context *ctx, const char* location);
int gst_duktape_start(duk_context *ctx, void *data);

int gst_duktape_render(duk_context *ctx, const char *buffer, size_t size, void *data);

#endif

