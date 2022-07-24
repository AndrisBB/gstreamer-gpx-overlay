/* GStreamer
 * Copyright (C) 2022 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_GPX_OVERLAY_H_
#define _GST_GPX_OVERLAY_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

#include <librsvg/rsvg.h>
#include "gstduktape.h"
#include "gpxparser.h"

G_BEGIN_DECLS

#define GST_TYPE_GPX_OVERLAY (gst_gpx_overlay_get_type())
#define GST_GPX_OVERLAY(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GPX_OVERLAY, GstGpxOverlay))
#define GST_GPX_OVERLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_GPX_OVERLAY, GstGpxOverlayClass))
#define GST_IS_GPX_OVERLAY(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GPX_OVERLAY))
#define GST_IS_GPX_OVERLAY_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_GPX_OVERLAY))

typedef struct _GstGpxOverlay GstGpxOverlay;
typedef struct _GstGpxOverlayClass GstGpxOverlayClass;

struct _GstGpxOverlay
{
	GstVideoFilter 	base_gpxoverlay;

	RsvgHandle 		*handle;

	duk_context 	*duk_ctx;
	gchar 			*gpx_location;

	gint 			offset_x;
	gint 			offset_y;

	gpx_trk_segment *segment;
};

struct _GstGpxOverlayClass
{
	GstVideoFilterClass base_gpxoverlay_class;
};

GType gst_gpx_overlay_get_type(void);

G_END_DECLS

#endif
