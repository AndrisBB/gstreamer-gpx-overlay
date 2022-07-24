#ifndef DUK_UTILS_H
#define DUK_UTILS_H

#include "duktape.h"

void * duk_utils_get_stack_pointer(duk_context *ctx, const char *type);
void duk_dump_object(duk_context *ctx, duk_idx_t idx);

#endif