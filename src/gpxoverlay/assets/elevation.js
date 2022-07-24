var document = new Document("../src/gpxoverlay/assets/elevation.svg");
this.Document = document;

var counter = 0;

var _segment = null;

var _elevation_min = Number.MAX_VALUE;
var _elevation_max = Number.MIN_VALUE;

var MAX_POINTS_TO_RENDER = 480;
var GRAPH_WIDTH = 480;
var GRAPH_HEIGHT = 240;

var _points_to_render = 0;

function start(segment)
{
    _segment = segment;

    // Find min and max elevation values
    var trkPoints = segment.trkPoints;
    for (var i = 0; i < trkPoints.length; i++) {
        if(trkPoints[i].elevation > _elevation_max) {
            _elevation_max = trkPoints[i].elevation;
        }
        if(trkPoints[i].elevation < _elevation_min) {
            _elevation_min = trkPoints[i].elevation;
        }
    }

    if(trkPoints.length >= MAX_POINTS_TO_RENDER) {
        _points_to_render = MAX_POINTS_TO_RENDER;
    }
    else {
        _points_to_render = trkPoints.length;
    }
}

function filter_points(points, idx) {
    
    var first_point = 0;

    if(idx < _points_to_render/2) {
        first_point = 0;
    }
    else {
        first_point = idx - _points_to_render/2;
    }

    return points.slice(first_point, first_point + _points_to_render);
}

function render(point)
{
    var points = filter_points(_segment.trkPoints, point.idx);
    
    var polygon = "480,240 0,240";

    for(var i = 0; i < points.length; i++) {
        var x = i * (GRAPH_WIDTH/points.length);
        var y = GRAPH_HEIGHT - parseInt(((_elevation_max - _elevation_min)/GRAPH_HEIGHT) * points[i].elevation);

        polygon = polygon.concat(" " + x + "," + y);

        if(point.idx == points[i].idx) {
            var el_pos = Document.getElementById("current_pos");
            if(el_pos != undefined) {
                el_pos.setAttribute("cx", x);
                el_pos.setAttribute("cy", y);
            }
        }
    }

    var el = Document.getElementById("elevation_graph");
    if(el != undefined) {
        el.setAttribute("points", polygon);
    }

    el = Document.getElementById("label_id");
    if(el != undefined) {
        el.innerHTML = "ELEVATION " + point.elevation.toFixed(2).toString() + "M";
    }

    return document.stringify();
}