#include "gpxparser.h"
#include <gst/gst.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <json-glib/json-glib.h>

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
			xmlFree(timestamp_str);
		}
	}

	return timestamp;
}

static gchar *
gpx_get_value_string(xmlNodePtr node)
{
	gchar *ptr = NULL;

	if(node != NULL) {
		xmlChar *xml_str = xmlNodeGetContent(node);
		if(xml_str != NULL) {
			ptr = g_strdup((const gchar *)xml_str);
			xmlFree(xml_str);
		}
	}

	return ptr;
}

static gpx_trk_point_t *
gpx_parse_trk_point(xmlNodePtr node) 
{
	if((node->type != XML_ELEMENT_NODE) || (strcmp((const char *)node->name, "trkpt") != 0)) {
		printf("[GPX Parser] Node is not trakpoint, skipping\n");
		return NULL;
	}

	gpx_trk_point_t *trk_pt = malloc(sizeof(gpx_trk_point_t));

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

	return trk_pt;
}

static gpx_trk_segment_t * 
gpx_parse_trk_segment(xmlNodePtr trk_seg_node)
{	
	gpx_trk_segment_t *segment = malloc(sizeof(gpx_trk_segment_t));
	segment->trk_points = NULL;

	gint point_idx = 0;

	xmlNodePtr trk_point_node = trk_seg_node->xmlChildrenNode;
	while(trk_point_node) {
		if((trk_point_node->type == XML_ELEMENT_NODE) && (strcmp((const char *)trk_point_node->name, "trkpt") == 0)) {
			gpx_trk_point_t *point = gpx_parse_trk_point(trk_point_node);
			if(point != NULL) {
				point->idx = point_idx++;
				segment->trk_points = g_list_append(segment->trk_points, (gpointer)point);
			}
		}
		trk_point_node = trk_point_node->next;
	}
	
	// if(g_list_length(segment->trk_points) > 0) {
	// 	gpx_trk_point_t *first = (gpx_trk_point_t *)segment->trk_points->data;
	// 	if(first == NULL) {
	// 		return segment;
	// 	}

	// 	segment->start_time = first->timestamp;

	// 	printf("[GPX Parser] Segment starts at:%s\n", g_date_time_format_iso8601(segment->start_time));
		
	// 	// Post process all the points and calculate offsets
	// 	GList *elem;
	// 	for(elem = segment->trk_points; elem; elem = elem->next) {
	// 		gpx_trk_point_t *point = (gpx_trk_point_t *)elem->data;
	// 		// printf("[GPX Parser] Point@:%s\n", g_date_time_format_iso8601(point->timestamp));

	// 		point->offset = g_date_time_difference(point->timestamp, segment->start_time) * 1000;
	// 		// printf("Offset: %" G_GINT64_FORMAT "\n", point->offset);
	// 	}
	// }

	return segment;
}

static gpx_trk_t * 
gpx_parse_trk(xmlNodePtr node)
{
	gpx_trk_t *trk = malloc(sizeof(gpx_trk_t));

	xmlNodePtr name_node = gpx_find_node(node, "name");
	if(name_node != NULL) {
		trk->name = gpx_get_value_string(name_node);
	}
	else {
		printf("[GPX Overlay] Trk does not have a name, skipping\n");
	}

	// At the moment we support only one segment
	xmlNodePtr seg_node = gpx_find_node(node, "trkseg");
	if(seg_node == NULL) {
		// We cannot do much if there is no segment
		printf("[GPX Overlay] Did not find 'trkseg'\n");
		goto segment_error;
	}

	gpx_trk_segment_t *trk_segment = gpx_parse_trk_segment(seg_node);
	if(trk_segment == NULL) {
		printf("[GPX Overlay] Failed to parse 'trkseg'\n");
		goto segment_error;
	}

	trk->segment = trk_segment;

	return trk;

segment_error:
	g_free(trk->name);
	free(trk);
	return NULL;
}

static GDateTime *
gpx_get_metadata_timestamp(xmlNodePtr node)
{
	if(node == NULL) {
		return NULL;
	}

	xmlNodePtr time_node = gpx_find_node(node, "time");
	if(time_node == NULL) {
		printf("[GPX Parser] Did not find time node\n");
		return NULL;
	}

	return gpx_get_value_timestamp(time_node);
}

gpx_t * 
gpx_parse_file(const char *location)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root = NULL;

	printf("[GPX Parser] Load file:%s\n", location);

	doc = xmlParseFile(location);
	if(doc == NULL) {
		printf("[GPX Parser] Failed to read GPX file\n");
		goto parser_error;
	}

	root = xmlDocGetRootElement(doc);
	if(root == NULL) {
		printf("[GPX Overlay] Failed to get GPX root\n");
		goto parser_error;
	}

	xmlNodePtr metadata = gpx_find_node(root, "metadata");
	if(metadata == NULL) {
		printf("[GPX Overlay] Did not find metadata node\n");
		goto parser_error;
	}

	GDateTime *metadata_time = gpx_get_metadata_timestamp(metadata);
	if(metadata_time == NULL) {
		printf("[GPX Overlay] Metadata does not have timestamp\n");
		goto parser_error;
	}

	// Fint a 'trk' node. We support only one trk per file
	xmlNodePtr trk_node = gpx_find_node(root, "trk");
	if(trk_node == NULL) {
		printf("[GPX Overlay] Did not find trk node\n");
		goto parser_error;
	}

	gpx_trk_t *trk = gpx_parse_trk(trk_node);
	if(trk == NULL) {
		printf("[GPX Overlay] Failed to parse 'trk' node\n");
		goto parser_error;
	}


	gpx_t *gpx_ptr = malloc(sizeof(gpx_ptr));
	gpx_ptr->time = metadata_time;
	gpx_ptr->trk = trk;

	return gpx_ptr;

parser_error:
	xmlFreeDoc(doc);
	return NULL;
}

gpx_trk_point_t * 
gpx_find_trk_point(gpx_t *gpx, gint64 offset, gint64 pts, gint64 duration)
{
	gpx_trk_point_t *point = NULL;

	GTimeSpan pts_offset = offset + (pts / 1000 / 1000); // pts is nanoseconds
	GDateTime *search_timestamp = g_date_time_add(gpx->time, pts_offset);

	// GST_INFO("Search for %s", g_date_time_format_iso8601(search_timestamp));
	GList *elem = NULL;
	for(elem = gpx->trk->segment->trk_points; elem; elem = elem->next) {
		gpx_trk_point_t *tmp = (gpx_trk_point_t *)elem->data;
		if(g_date_time_difference(tmp->timestamp, search_timestamp) >= 0) {
			break;
		}
	}

	if(elem != NULL && elem->prev != NULL) {
		elem = elem->prev;
		gpx_trk_point_t *tmp = (gpx_trk_point_t *)elem->data;
		// GST_INFO("Point at:%s", g_date_time_format_iso8601(tmp->timestamp));
		point = tmp;
	}
	
	return point;
}

gchar *
gpx_trk_point_json(gpx_trk_point_t *point)
{
	JsonBuilder *builder = json_builder_new();

	json_builder_begin_object(builder);

	json_builder_set_member_name(builder, "timestamp");
	json_builder_add_string_value(builder, g_date_time_format_iso8601(point->timestamp));

	json_builder_set_member_name(builder, "lat");
	json_builder_add_double_value(builder, point->lat);

	json_builder_set_member_name(builder, "lon");
	json_builder_add_double_value(builder, point->lon);

	json_builder_set_member_name(builder, "elevation");
	json_builder_add_double_value(builder, point->elevation);

	json_builder_set_member_name(builder, "hr");
	json_builder_add_int_value(builder, point->hr);

	json_builder_end_object(builder);

	JsonNode *node = json_builder_get_root(builder);
	g_object_unref(builder);

	JsonGenerator *generator = json_generator_new();
	json_generator_set_root(generator, node);
	
	gsize length;
  	gchar *data;
	data = json_generator_to_data(generator, &length);

	json_node_free(node);
	g_object_unref(generator);

	return data;
}

void
gpx_dump_point(gpx_trk_point_t *point)
{
	if(point != NULL) {
		GST_INFO("----------------------------------");
		GST_INFO("Timestamp:%s", g_date_time_format_iso8601(point->timestamp));
		GST_INFO("Latitude:  %.6f", point->lat);
		GST_INFO("Longitude: %.6f", point->lon);
		GST_INFO("Elevation: %.6f", point->elevation);
		GST_INFO("HR: %d", point->hr);
	}
}

void
gpx_dump(gpx_t *ptr)
{
	GST_INFO("----------------------------");
	GST_INFO("GPX Dump");
	GST_INFO("----------------------------");
	GST_INFO("Time:%s", g_date_time_format_iso8601(ptr->time));
	GST_INFO("Track Name:%s", ptr->trk->name);

	if(ptr->trk->segment->trk_points != NULL  && g_list_length(ptr->trk->segment->trk_points) > 0) {
		GList *elem;
		for(elem = ptr->trk->segment->trk_points; elem; elem = elem->next) {
			gpx_trk_point_t *point = (gpx_trk_point_t *)elem->data;
			GST_INFO("Point@:%s", g_date_time_format_iso8601(point->timestamp));

			// point->offset = g_date_time_difference(point->timestamp, segment->start_time) * 1000;
			// printf("Offset: %" G_GINT64_FORMAT "\n", point->offset);
		}
	}
}