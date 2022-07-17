#include <gst/gst.h>

#include "gstgpxparser.h"
#include "gstgpxoverlay.h"

static gboolean
plugin_init(GstPlugin *plugin)
{
    gboolean ret = FALSE;

    ret |= gst_element_register(plugin, "gpxoverlay", GST_RANK_NONE,
                                GST_TYPE_GPX_OVERLAY);

    ret |= gst_element_register(plugin, "gpxparser", GST_RANK_NONE,
                                GST_TYPE_GPX_PARSER);

    return ret;
}

#ifndef VERSION
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  gpxoverlay,
                  "FIXME plugin description",
                  plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

