#include "mod_dom.h"
#include "document.h"
#include "element.h"
#include "trk_point.h"
#include "trk_segment.h"

#include <stdio.h>
#include <gdome.h>

duk_ret_t dukopen_dom_module(duk_context *ctx)
{
    document_init(ctx);
    element_init(ctx);
    trk_point_init(ctx);
    trk_segment_init(ctx);

    return 1;
}



