#include "mod_dom.h"
#include "document.h"
#include "element.h"
#include "trk_point.h"

#include <stdio.h>
#include <gdome.h>

// static duk_ret_t element_set_inner_html(duk_context *ctx)
// {
//     const char *html = duk_safe_to_string(ctx, -1);
//     printf("Set inner html to:%s\n", html);

//     return 0;
// }

// static duk_ret_t element_get_inner_html(duk_context *ctx)
// {
//     printf("Get inner html\n");
//     duk_push_string(ctx, "Test inner HTML");
//     return 0;
// }

duk_ret_t dukopen_dom_module(duk_context *ctx)
{
    document_init(ctx);
    element_init(ctx);
    trk_point_init(ctx);

    return 1;
}



