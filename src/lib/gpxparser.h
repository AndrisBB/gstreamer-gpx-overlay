#ifndef GPX_PARSER_H
#define GPX_PARSER_H

#include <glib.h>

typedef struct
{
    gint idx;

    GDateTime *timestamp;

    gfloat lat;
    gfloat lon;
    gfloat elevation;

    gint hr;
}
gpx_trk_point_t;

typedef struct
{
    GList *trk_points;
}
gpx_trk_segment_t;

typedef struct
{
    gchar *                 name;
    gpx_trk_segment_t *     segment;
} 
gpx_trk_t;

typedef struct 
{
    GDateTime *             time;
    gpx_trk_t *             trk;
} 
gpx_t;


gpx_t *                 gpx_parse_file(const char *location);
gpx_trk_point_t *       gpx_find_trk_point(gpx_t *gpx, gint64 offset, gint64 pts, gint64 duration);

gchar *                 gpx_trk_point_json(gpx_trk_point_t *point);

void                    gpx_dump(gpx_t *ptr);

#endif