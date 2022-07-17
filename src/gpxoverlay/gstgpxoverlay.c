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
 * SECTION:element-gstgpxoverlay
 *
 * The gpxoverlay element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! gpxoverlay ! FIXME ! fakesink
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
#include "gstgpxoverlay.h"

#include "gstduktape.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

#define MAX_SVG_BUFFER_SIZE	65536


GST_DEBUG_CATEGORY_STATIC(gst_gpx_overlay_debug_category);
#define GST_CAT_DEFAULT gst_gpx_overlay_debug_category

/* prototypes */

static void gst_gpx_overlay_set_property(GObject *object,
										 guint property_id, const GValue *value, GParamSpec *pspec);
static void gst_gpx_overlay_get_property(GObject *object,
										 guint property_id, GValue *value, GParamSpec *pspec);
static void gst_gpx_overlay_dispose(GObject *object);
static void gst_gpx_overlay_finalize(GObject *object);

static gboolean gst_gpx_overlay_start(GstBaseTransform *trans);
static gboolean gst_gpx_overlay_stop(GstBaseTransform *trans);
static gboolean gst_gpx_overlay_set_info(GstVideoFilter *filter, GstCaps *incaps,
										 GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info);

static GstFlowReturn gst_gpx_overlay_transform_frame_ip(GstVideoFilter *filter,
														GstVideoFrame *frame);

enum
{
	PROP_0,
	PROP_LOCATION,
	PROP_SCRIPT,
	PROP_GPX_LOCATION
};

/* pad templates */

/* FIXME: add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
	GST_VIDEO_CAPS_MAKE("{ BGRA }")

/* FIXME: add/remove formats you can handle */
#define VIDEO_SINK_CAPS \
	GST_VIDEO_CAPS_MAKE("{ BGRA }")

/* class initialization */

G_DEFINE_TYPE_WITH_CODE(GstGpxOverlay, gst_gpx_overlay, GST_TYPE_VIDEO_FILTER,
						GST_DEBUG_CATEGORY_INIT(gst_gpx_overlay_debug_category, "gpxoverlay", 0,
												"debug category for gpxoverlay element"));

static void
gst_gpx_overlay_set_svg_data(GstGpxOverlay *gpxoverlay, const gchar *location)
{
	GError *error = NULL;

	GST_WARNING_OBJECT(gpxoverlay, "Set SVG location to:%s", location);

	if(location == NULL) {
		GST_ERROR_OBJECT(gpxoverlay, "SVG location is NULL");
		return;
	}

	if(strlen(location) == 0) {
		GST_ERROR_OBJECT(gpxoverlay, "SVG location is empty");
		return;
	}

	if(gpxoverlay->handle != NULL) {
		GST_INFO_OBJECT(gpxoverlay, "Reseting RSVG handle");
		g_object_unref (gpxoverlay->handle);
		gpxoverlay->handle = NULL;
	}

	gpxoverlay->handle = rsvg_handle_new_from_file(location, &error);

	if(error || gpxoverlay->handle == NULL) {
		GST_ERROR_OBJECT(gpxoverlay, "Failed to load SVG:%s", error->message);
		g_error_free(error);
		return;
	}

	RsvgDimensionData svg_dimension;
	rsvg_handle_get_dimensions(gpxoverlay->handle, &svg_dimension);

	GST_WARNING_OBJECT(gpxoverlay, "Loaded SVG data, dimensions %d x %d", svg_dimension.width, svg_dimension.height);

	// Read SVG contents into memory
	// GError* err = NULL;
	// printf("Read SVG template\n");
	// gboolean success = g_file_get_contents(location, &gpxoverlay->svg_template, &gpxoverlay->svg_template_length, &err);
	// if(!success) {
	// 	printf("Failed to read SVG template\n");
	// }
	// else {
	// 	printf("%s\n", gpxoverlay->svg_template);
	// }

	return;
}

static void
gst_gpx_overlay_class_init(GstGpxOverlayClass *klass)
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

	gobject_class->set_property = gst_gpx_overlay_set_property;
	gobject_class->get_property = gst_gpx_overlay_get_property;
	gobject_class->dispose = gst_gpx_overlay_dispose;
	gobject_class->finalize = gst_gpx_overlay_finalize;
	base_transform_class->start = GST_DEBUG_FUNCPTR(gst_gpx_overlay_start);
	base_transform_class->stop = GST_DEBUG_FUNCPTR(gst_gpx_overlay_stop);
	video_filter_class->set_info = GST_DEBUG_FUNCPTR(gst_gpx_overlay_set_info);
	video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR(gst_gpx_overlay_transform_frame_ip);

	g_object_class_install_property (	G_OBJECT_CLASS (klass), PROP_LOCATION,
										g_param_spec_string ("location", "location", "SVG file location.", "",
										G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (	G_OBJECT_CLASS (klass), PROP_SCRIPT,
										g_param_spec_string ("script", "script", "JavaScript location.", "",
										G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (	G_OBJECT_CLASS (klass), PROP_GPX_LOCATION,
										g_param_spec_string ("gpx-location", "gpx-location", "GPX file to parse", "",
										G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));
}

static void
gst_gpx_overlay_init(GstGpxOverlay *gpxoverlay)
{
	GST_WARNING_OBJECT(gpxoverlay, "gst_gpx_overlay_init");

	gpxoverlay->handle = NULL;
	gpxoverlay->duk_ctx = NULL;
	gpxoverlay->svg_template = NULL;
	gpxoverlay->svg_template_length = 0;

	gpxoverlay->gpx_location = NULL;
	gpxoverlay->segment = NULL;

	gst_duktape_init(&gpxoverlay->duk_ctx);
}

void gst_gpx_overlay_set_property(GObject *object, guint property_id,
								  const GValue *value, GParamSpec *pspec)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(object);

	GST_WARNING_OBJECT(gpxoverlay, "gst_gpx_overlay_set_property");

	switch (property_id)
	{
		case PROP_LOCATION:
		{
			gst_gpx_overlay_set_svg_data(gpxoverlay, g_value_get_string(value));
			break;
		}
		case PROP_SCRIPT:
		{
			gst_duktape_load_script(gpxoverlay->duk_ctx, g_value_get_string(value));
			break;
		}
		case PROP_GPX_LOCATION:
		{
			gpxoverlay->gpx_location = g_value_dup_string(value);
			break;
		}
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

void gst_gpx_overlay_get_property(GObject *object, guint property_id,
								  GValue *value, GParamSpec *pspec)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(object);

	GST_DEBUG_OBJECT(gpxoverlay, "get_property");

	switch (property_id)
	{
		case PROP_GPX_LOCATION:
		{
			g_value_set_string(value, gpxoverlay->gpx_location);
			break;
		}
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
			break;
	}
}

void gst_gpx_overlay_dispose(GObject *object)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(object);

	GST_DEBUG_OBJECT(gpxoverlay, "dispose");

	/* clean up as possible.  may be called multiple times */

	G_OBJECT_CLASS(gst_gpx_overlay_parent_class)->dispose(object);
}

void gst_gpx_overlay_finalize(GObject *object)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(object);

	GST_DEBUG_OBJECT(gpxoverlay, "finalize");

	/* clean up object here */

	G_OBJECT_CLASS(gst_gpx_overlay_parent_class)->finalize(object);
}

static gboolean
gst_gpx_overlay_start(GstBaseTransform *trans)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(trans);

	printf("[GPX Overlay] Start\n");

	GST_WARNING_OBJECT(gpxoverlay, "start");

	gst_duktape_start(gpxoverlay->duk_ctx);

	// Load GPX file
	printf("[GPX Overlay] GPX file:%s\n", gpxoverlay->gpx_location);
	gpxoverlay->segment = gpx_parse_file(gpxoverlay->gpx_location);
	if(gpxoverlay->segment == NULL) {
		printf("[GPX Overlay] Failed to parse GPX\n");
		return FALSE;
	}

	return TRUE;
}

static gboolean
gst_gpx_overlay_stop(GstBaseTransform *trans)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(trans);

	GST_WARNING_OBJECT(gpxoverlay, "stop");

	return TRUE;
}

static gboolean
gst_gpx_overlay_set_info(GstVideoFilter *filter, GstCaps *incaps,
						 GstVideoInfo *in_info, GstCaps *outcaps, GstVideoInfo *out_info)
{
	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(filter);

	GST_WARNING_OBJECT(gpxoverlay, "-------------------------------------------");

	GST_WARNING_OBJECT(gpxoverlay, "set_info");
	GST_WARNING_OBJECT(gpxoverlay, "Incomming caps are %" GST_PTR_FORMAT, incaps);
	GST_WARNING_OBJECT(gpxoverlay, "Outgoing caps are %" GST_PTR_FORMAT, outcaps);

	
	GST_WARNING_OBJECT(gpxoverlay, "Input format:");
	GST_WARNING_OBJECT(gpxoverlay, "Format:  %s", gst_video_format_to_string(GST_VIDEO_INFO_FORMAT(in_info)));
	GST_WARNING_OBJECT(gpxoverlay, "Width:   %d", GST_VIDEO_INFO_WIDTH(in_info));
	GST_WARNING_OBJECT(gpxoverlay, "Height:  %d", GST_VIDEO_INFO_HEIGHT(in_info));
	GST_WARNING_OBJECT(gpxoverlay, "Size:  %d", (gint)GST_VIDEO_INFO_SIZE(in_info));
	GST_WARNING_OBJECT(gpxoverlay, "Offset:  %d", (gint)*in_info->offset);
	GST_WARNING_OBJECT(gpxoverlay, "Stride:  %d", (gint)*in_info->stride);

	GST_WARNING_OBJECT(gpxoverlay, "-------------------------------------------");

	return TRUE;
}

static GstFlowReturn
gst_gpx_overlay_transform_frame_ip(GstVideoFilter *filter, GstVideoFrame *frame)
{
	char svg_buffer[MAX_SVG_BUFFER_SIZE];

	GstGpxOverlay *gpxoverlay = GST_GPX_OVERLAY(filter);

	if(gpxoverlay->duk_ctx == NULL) {
		GST_WARNING_OBJECT(gpxoverlay, "Duktape ctx is null");
		return GST_FLOW_ERROR;
	}

	// Find GPX datapoint to pass to renderer
	// printf("================================\n");
	// printf("Frame PTS: %" G_GINT64_FORMAT "\n", GST_BUFFER_PTS(frame->buffer));
	// printf("Frame DURATION: %" G_GINT64_FORMAT "\n", GST_BUFFER_DURATION(frame->buffer));
	
	gpx_trk_point *point = gpx_find_trk_point(gpxoverlay->segment, GST_BUFFER_PTS(frame->buffer), GST_BUFFER_DURATION(frame->buffer));
	// gpx_dump_point(point);

	int data_len = gst_duktape_render(gpxoverlay->duk_ctx, svg_buffer, MAX_SVG_BUFFER_SIZE, (void *)point);

	if(gpxoverlay->handle == NULL) {
		GST_WARNING_OBJECT(gpxoverlay, "No RSV data loaded, nothing to do");
		return GST_FLOW_ERROR;
	}

	// Load returned string into RSVG
    GError *error = NULL;
	RsvgHandle *handle = rsvg_handle_new_from_data((const guint8*)svg_buffer, data_len, &error);
	if(error || handle == NULL) {
		printf("Failed to load SVG:%s", error->message);
		g_error_free(error);
		return GST_FLOW_ERROR;
	}

	RsvgDimensionData svg_dimension;
	rsvg_handle_get_dimensions(handle, &svg_dimension);

	// printf("Loaded SVG data, dimensions %d x %d\n", svg_dimension.width, svg_dimension.height);


	cairo_surface_t *surface;
	cairo_t *cr;

	surface = cairo_image_surface_create_for_data(	GST_VIDEO_FRAME_PLANE_DATA(frame, 0), 
													CAIRO_FORMAT_ARGB32, 
													GST_VIDEO_FRAME_WIDTH(frame),
													GST_VIDEO_FRAME_HEIGHT(frame), 
													GST_VIDEO_FRAME_PLANE_STRIDE(frame, 0));
	
	if(G_UNLIKELY(!surface)) {
		GST_ERROR_OBJECT(gpxoverlay, "Failed to create surface for Cairo");
		return GST_FLOW_ERROR;
	}
		
	cr = cairo_create(surface);
	if(G_UNLIKELY (!cr)) {
		GST_ERROR_OBJECT(gpxoverlay, "Failed to create Cairo context");
		cairo_surface_destroy(surface);
		return GST_FLOW_ERROR;
	}

	cairo_scale (cr, (double) 1.0, (double) 1);

	rsvg_handle_render_cairo(handle, cr);

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	return GST_FLOW_OK;
}
