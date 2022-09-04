# gstreamer-overlay

## Test pipelines

```
gst-launch-1.0 videotestsrc \
    ! video/x-raw,width=1280,height=960 \
    ! gpxoverlay \
        script-location=../src/gpxoverlay/assets/elevation.js \
        gpx-location=../src/gpxoverlay/assets/route.gpx \
    ! videoconvert \
    ! autovideosink
```


```
gst-launch-1.0 videotestsrc \
    ! video/x-raw,width=640,height=480,framerate=1/1 \
    ! gpxparser \
        location=../src/gpxoverlay/assets/route.gpx \
    ! fakesink
```


