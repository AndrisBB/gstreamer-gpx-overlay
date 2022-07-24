#include "gpxparser.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

static xmlNodePtr
gpx_find_node(xmlNodePtr root, const char *name)
{
	xmlNodePtr cur = root->xmlChildrenNode;
	while(cur) {
		if((cur->type == XML_ELEMENT_NODE) && (strcmp((const char *)cur->name, name) == 0)) {
			break;;
		}
		cur = cur->next;
	}

	return cur;
}

static gfloat 
gpx_get_attribute_float(xmlNodePtr node, const char *attr)
{
	gfloat value = 0.0;

	xmlChar *attr_val = NULL;
	xmlChar *attr_name = xmlCharStrdup(attr);

	attr_val = xmlGetProp(node, attr_name);
	
	if(attr_val != NULL) {
		value = atof((const char *)attr_val);
	}

	xmlFree(attr_val);
	xmlFree(attr_name);

	return value;
}

static gfloat 
gpx_get_value_float(xmlNodePtr node)
{
	gfloat value = 0.0f;
	if(node != NULL) {
		xmlChar *value_str = xmlNodeGetContent(node);
		if(value_str != NULL) {
			value = atof((const char *)value_str);
		}
		xmlFree(value_str);
	}

	return value;
}

static gint 
gpx_get_value_int(xmlNodePtr node)
{
	gint value = 0;
	if(node != NULL) {
		xmlChar *value_str = xmlNodeGetContent(node);
		if(value_str != NULL) {
			value = atoi((const char *)value_str);
		}
		xmlFree(value_str);
	}

	return value;
}

static GDateTime *
gpx_get_value_timestamp(xmlNodePtr node)
{
	GDateTime *timestamp = NULL;

	if(node != NULL) {
		xmlChar *timestamp_str = xmlNodeGetContent(node);
		if(timestamp_str != NULL) {
			timestamp = g_date_time_new_from_iso8601((const char *)timestamp_str, NULL);

			// printf("[GPX Parser] Got time:%s\n", g_date_time_format_iso8601(timestamp));

			xmlFree(timestamp_str);
		}
	}

	return timestamp;
}

static gpx_trk_point *
gpx_parse_trk_point(xmlNodePtr node) 
{
	if((node->type != XML_ELEMENT_NODE) || (strcmp((const char *)node->name, "trkpt") != 0)) {
		printf("[GPX Parser] Node is not trakpoint, skipping\n");
		return NULL;
	}

	gpx_trk_point *trk_pt = malloc(sizeof(gpx_trk_point));

	// lat/lon
	trk_pt->lat = gpx_get_attribute_float(node, "lat");
	trk_pt->lon = gpx_get_attribute_float(node, "lon");

	// elevation
	xmlNodePtr elevation_node = gpx_find_node(node, "ele");
	if(elevation_node != NULL) {
		trk_pt->elevation = gpx_get_value_float(elevation_node);
	}

	// timestamp
	xmlNodePtr time_node = gpx_find_node(node, "time");
	if(time_node != NULL) {
		trk_pt->timestamp = gpx_get_value_timestamp(time_node);
	}
	
	// Heat Rate
	xmlNodePtr extensions_node = gpx_find_node(node, "extensions");
	if(extensions_node != NULL) {
		xmlNodePtr TrackPointExtension = gpx_find_node(extensions_node, "TrackPointExtension");
		if(TrackPointExtension != NULL) {
			xmlNodePtr hr_node = gpx_find_node(TrackPointExtension, "hr");
			if(hr_node != NULL) {
				trk_pt->hr = gpx_get_value_int(hr_node);
			}
		}
	}


	// printf("Latitude:  %.6f\n", trk_pt->lat);
	// printf("Longitude: %.6f\n", trk_pt->lon);
	// printf("Elevation: %.6f\n", trk_pt->elevation);

	return trk_pt;
}

static gpx_trk_segment * 
gpx_parse_trk_segment(xmlNodePtr trk_seg_node)
{
	printf("[GPX Parser] Parse segment\n");
	
	gpx_trk_segment *segment = malloc(sizeof(gpx_trk_segment));

	gint point_idx = 0;

	xmlNodePtr trk_point_node = trk_seg_node->xmlChildrenNode;
	while(trk_point_node) {
		if((trk_point_node->type == XML_ELEMENT_NODE) && (strcmp((const char *)trk_point_node->name, "trkpt") == 0)) {
			gpx_trk_point *point = gpx_parse_trk_point(trk_point_node);
			if(point != NULL) {
				point->idx = point_idx++;
				segment->trk_points = g_list_append(segment->trk_points, (gpointer)point);
			}
		}
		trk_point_node = trk_point_node->next;
	}

	printf("[GPX Parser] Segment contains %u points\n", g_list_length(segment->trk_points));
	
	if(g_list_length(segment->trk_points) > 0) {
		gpx_trk_point *first = (gpx_trk_point *)segment->trk_points->data;
		if(first == NULL) {
			return segment;
		}

		segment->start_time = first->timestamp;

		printf("[GPX Parser] Segment starts at:%s\n", g_date_time_format_iso8601(segment->start_time));
		
		// Post process all the points and calculate offsets
		GList *elem;
		for(elem = segment->trk_points; elem; elem = elem->next) {
			gpx_trk_point *point = (gpx_trk_point *)elem->data;
			// printf("[GPX Parser] Point@:%s\n", g_date_time_format_iso8601(point->timestamp));

			point->offset = g_date_time_difference(point->timestamp, segment->start_time) * 1000;
			// printf("Offset: %" G_GINT64_FORMAT "\n", point->offset);
		}
	}

	return segment;
}

gpx_trk_segment * 
gpx_parse_file(const char *location)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;

	doc = xmlParseFile(location);
	if(doc == NULL) {
		printf("[GPX Overlay] Failed to read GPX file\n");
		return FALSE;
	}

	root = xmlDocGetRootElement(doc);
	if(root == NULL) {
		printf("[GPX Overlay] Failed to get GPX root\n");
		xmlFreeDoc(doc);
		return FALSE;
	}

	// Find metadata trk
	xmlNodePtr trk_node = gpx_find_node(root, "trk");
	if(trk_node == NULL) {
		printf("[GPX Overlay] Did not faild TRK node\n");
		return FALSE;
	}

	// Find first segment
	xmlNodePtr trk_seg_node = gpx_find_node(trk_node, "trkseg");
	if(trk_seg_node == NULL) {
		printf("[GPX Overlay] Did not faild trkseg node\n");
		return FALSE;
	}

	gpx_trk_segment *segment = gpx_parse_trk_segment(trk_seg_node);

	return segment;
}

gpx_trk_point * gpx_find_trk_point(gpx_trk_segment *segment, gint64 pts, gint64 duration)
{
	gpx_trk_point *point = NULL;
	GList *elem = NULL;

	// printf("find point between %" G_GINT64_FORMAT " and %" G_GINT64_FORMAT "\n", pts, pts + duration);

	for(elem = segment->trk_points; elem; elem = elem->next) {
		gpx_trk_point *tmp = (gpx_trk_point *)elem->data;
		// printf("Offset: %" G_GINT64_FORMAT "\n", point->offset);
		point = tmp;
		if(tmp->offset > pts) {
			// printf("match\n");
			// point = tmp;
			break;
		}
	}

	return point;
}

void
gpx_dump_point(gpx_trk_point *point)
{
	if(point != NULL) {
		printf("----------------------------------\n");
		printf("Timestamp:%s\n", g_date_time_format_iso8601(point->timestamp));
		printf("Offset: %" G_GINT64_FORMAT "\n", point->offset);
		printf("Latitude:  %.6f\n", point->lat);
		printf("Longitude: %.6f\n", point->lon);
		printf("Elevation: %.6f\n", point->elevation);
		printf("HR: %d\n", point->hr);
	}
}