project('gstreamer-gpx-overlay', 'c')

incdir = include_directories(
  'src',
  'src/lib',
  'src/plugins/gpxparser', 
  'src/plugins/gpxoverlay', 
  'src/plugins/gpxoverlay/mod_dom')

shared_library(
  'gstgpxoverlay',
  'src/gstgpx.c',
  'src/lib/overlaymetadata.c',
  'src/lib/gpxparser.c',
  'src/plugins/gpxparser/gstgpxparser.c',
  'src/plugins/gpxoverlay/gstgpxoverlay.c',
  'src/plugins/gpxoverlay/gstduktape.c',
  'src/plugins/gpxoverlay/duktape.c',
  'src/plugins/gpxoverlay/mod_dom/duk_utils.c',
  'src/plugins/gpxoverlay/mod_dom/mod_dom.c',
  'src/plugins/gpxoverlay/mod_dom/document.c',
  'src/plugins/gpxoverlay/mod_dom/element.c',
  'src/plugins/gpxoverlay/mod_dom/trk_point.c',
  'src/plugins/gpxoverlay/mod_dom/trk_segment.c',
  include_directories : incdir,
  name_prefix : '',
  dependencies : [
    dependency('gstreamer-1.0', version : '>=1.16.0'),
    dependency('gstreamer-video-1.0'),
    dependency('librsvg-2.0', version : '>= 2.36.2'),
    dependency('libxml-2.0'),
    dependency('json-glib-1.0')
  ]
)
