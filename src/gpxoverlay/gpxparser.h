#ifndef GPX_PARSER_H
#define GPX_PARSER_H

#include <glib.h>

typedef struct
{
    GDateTime *timestamp;
    gint64 offset;

    gfloat lat;
    gfloat lon;
    gfloat elevation;

    gint hr;
}
gpx_trk_point;

typedef struct
{
    GDateTime *start_time;
    GList *trk_points;
}
gpx_trk_segment;


gpx_trk_segment *   gpx_parse_file(const char *location);
gpx_trk_point *     gpx_find_trk_point(gpx_trk_segment *segment, gint64 pts, gint64 duration);


void                gpx_dump_point(gpx_trk_point *point);

#endif