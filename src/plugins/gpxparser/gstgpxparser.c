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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstgpxparser
 *
 * The gpxparser element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! gpxparser ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gstgpxparser.h"
#include "overlaymetadata.h"

#include <stdio.h>

GST_DEBUG_CATEGORY_STATIC(gst_gpx_parser_debug_category);
#define GST_CAT_DEFAULT gst_gpx_parser_debug_category

/* prototypes */

static void gst_gpx_parser_set_property(GObject *object,
										guint property_id, const GValue *value, GParamSpec *pspec);
static void gst_gpx_parser_get_property(GObject *object,
										guint property_id, GValue *value, GParamSpec *pspec);
static void gst_gpx_parser_dispose(GObject *object);
static void gst_gpx_parser_finalize(GObject *object);

static gboolean gst_gpx_parser_start(GstBaseTransform *trans);
static gboolean gst_gpx_parser_stop(GstBaseTransform *trans);
static gboolean gst_gpx_parser_set_info(GstVideoFilter *filter, GstCaps *incaps,
										GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info);
static GstFlowReturn gst_gpx_parser_transform_frame_ip(GstVideoFilter *filter,
													   GstVideoFrame *frame);

enum
{
	PROP_0,
	PROP_LOCATION
};


#define VIDEO_SRC_CAPS 		GST_VIDEO_CAPS_MAKE("{ BGRA }")
#define VIDEO_SINK_CAPS 	GST_VIDEO_CAPS_MAKE("{ BGRA }")


G_DEFINE_TYPE_WITH_CODE(GstGpxParser, gst_gpx_parser, GST_TYPE_VIDEO_FILTER,
						GST_DEBUG_CATEGORY_INIT(gst_gpx_parser_debug_category, "gpxparser", 0,
												"debug category for gpxparser element"));

static void
gst_gpx_parser_class_init(GstGpxParserClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
	GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);
	GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS(klass);

	/* Setting up pads and setting metadata should be moved to
	   base_class_init if you intend to subclass this class. */
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
									   gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS,
															gst_caps_from_string(VIDEO_SRC_CAPS)));
	gst_element_class_add_pad_template(GST_ELEMENT_CLASS(klass),
									   gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
															gst_caps_from_string(VIDEO_SINK_CAPS)));

	gst_element_class_set_static_metadata(GST_ELEMENT_CLASS(klass),
										  "FIXME Long name", "Generic", "FIXME Description",
										  "FIXME <fixme@example.com>");

	gobject_class->set_property = gst_gpx_parser_set_property;
	gobject_class->get_property = gst_gpx_parser_get_property;
	gobject_class->dispose = gst_gpx_parser_dispose;
	gobject_class->finalize = gst_gpx_parser_finalize;
	base_transform_class->start = GST_DEBUG_FUNCPTR(gst_gpx_parser_start);
	base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_gpx_parser_stop);
	video_filter_class->set_info = GST_DEBUG_FUNCPTR(gst_gpx_parser_set_info);
	video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR(gst_gpx_parser_transform_frame_ip);

	g_object_class_install_property (	G_OBJECT_CLASS (klass), PROP_LOCATION,
										g_param_spec_string ("location", "location", "GPX file location.", "",
										G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
}

static void
gst_gpx_parser_init(GstGpxParser *gpxparser)
{
	gpxparser->location = NULL;
	gpxparser->gpx = NULL;
}

void gst_gpx_parser_set_property(GObject *object, guint property_id,
								 const GValue *value, GParamSpec *pspec)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(object);

	switch (property_id)
	{
		case PROP_LOCATION:
			if(gpxparser->location != NULL) {
				g_free(gpxparser->location);
			}
			gpxparser->location = g_value_dup_string(value);
			printf("set location to:%s\n", gpxparser->location);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
		}
}

void gst_gpx_parser_get_property(GObject *object, guint property_id,
								 GValue *value, GParamSpec *pspec)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(object);

	GST_DEBUG_OBJECT(gpxparser, "get_property");

	switch (property_id)
	{
		case PROP_LOCATION:
			g_value_set_string(value, gpxparser->location);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

void gst_gpx_parser_dispose(GObject *object)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(object);

	GST_DEBUG_OBJECT(gpxparser, "dispose");

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS(gst_gpx_parser_parent_class)->dispose(object);
}

void gst_gpx_parser_finalize(GObject *object)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(object);

	GST_DEBUG_OBJECT(gpxparser, "finalize");

	/* clean up object here */

	G_OBJECT_CLASS(gst_gpx_parser_parent_class)->finalize(object);
}

static gboolean
gst_gpx_parser_start(GstBaseTransform *trans)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(trans);

	if(gpxparser->location != NULL) {
		gpx_t *gpx = gpx_parse_file(gpxparser->location);
		if(gpx != NULL) {
			gpx_dump(gpx);
			gpxparser->gpx = gpx;
		}
	}
	else {
		GST_WARNING_OBJECT(gpxparser, "Location is not set, so will do nothing\n");
	}

	return TRUE;
}

static gboolean
gst_gpx_parser_stop(GstBaseTransform *trans)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(trans);

	GST_DEBUG_OBJECT(gpxparser, "stop");

	return TRUE;
}

static gboolean
gst_gpx_parser_set_info(GstVideoFilter *filter, GstCaps *incaps,
						GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(filter);

	GST_DEBUG_OBJECT(gpxparser, "set_info");

	return TRUE;
}

static GstFlowReturn
gst_gpx_parser_transform_frame_ip(GstVideoFilter *filter, GstVideoFrame *frame)
{
	GstGpxParser *gpxparser = GST_GPX_PARSER(filter);

	// GST_INFO_OBJECT(gpxparser, "Frame PTS: %" G_GINT64_FORMAT, GST_BUFFER_PTS(frame->buffer));
	// printf("Frame DURATION: %" G_GINT64_FORMAT "\n", GST_BUFFER_DURATION(frame->buffer));
	
	gpx_trk_point_t *point = gpx_find_trk_point(gpxparser->gpx, 90000000, GST_BUFFER_PTS(frame->buffer), GST_BUFFER_DURATION(frame->buffer));
	if(point != NULL) {
		char *json = gpx_trk_point_json(point);
		gst_buffer_add_overlay_meta(frame->buffer, json);
	}
	
	return GST_FLOW_OK;
}
