#include "duk_utils.h"

void * duk_utils_get_stack_pointer(duk_context *ctx, const char *type)
{
    duk_push_this(ctx);
    duk_get_prop_string(ctx, -1, type);

    void *ptr = duk_to_pointer(ctx, -1);

    duk_pop(ctx);

    return ptr;
}

void duk_dump_object(duk_context *ctx, duk_idx_t idx) 
{
	idx = duk_require_normalize_index(ctx, idx);

	/* The weird fn() helper is to handle lightfunc name printing (= avoid it). */
	duk_eval_string(ctx,
	    "(function (o) {\n"
	    "    Object.getOwnPropertyNames(o).forEach(function (k) {\n"
	    "        var pd = Object.getOwnPropertyDescriptor(o, k);\n"
	    "        function fn(x) { if (x.name !== 'getter' && x.name !== 'setter') { return 'func' }; return x.name; }\n"
	    "        print(Duktape.enc('jx', k), Duktape.enc('jx', pd), (pd.get ? fn(pd.get) : 'no-getter'), (pd.set ? fn(pd.set) : 'no-setter'));\n"
	    "    });\n"
	    "})");
	duk_dup(ctx, idx);
	duk_call(ctx, 1);
	duk_pop(ctx);
}