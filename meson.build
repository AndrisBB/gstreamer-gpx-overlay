project('gstreamer-gpx-overlay', 'c')

incdir = include_directories('src', 'src/gpxparser', 'src/gpxoverlay', 'src/gpxoverlay/mod_dom')

shared_library('gstgpxoverlay',
  'src/gstgpx.c',
  'src/gpxparser/gstgpxparser.c',
  'src/gpxoverlay/gstgpxoverlay.c',
  'src/gpxoverlay/gpxparser.c',
  'src/gpxoverlay/gstduktape.c',
  'src/gpxoverlay/duktape.c',
  'src/gpxoverlay/mod_dom/duk_utils.c',
  'src/gpxoverlay/mod_dom/mod_dom.c',
  'src/gpxoverlay/mod_dom/document.c',
  'src/gpxoverlay/mod_dom/element.c',
  'src/gpxoverlay/mod_dom/trk_point.c',
  'src/gpxoverlay/mod_dom/trk_segment.c',
  include_directories : incdir,
  name_prefix : '',
  dependencies : [
    dependency('gstreamer-1.0', version : '>=1.16.0'),
    dependency('gstreamer-video-1.0'),
    dependency('librsvg-2.0', version : '>= 2.36.2'),
    dependency('libxml-2.0'),
    dependency('gdome2')
  ]
)
