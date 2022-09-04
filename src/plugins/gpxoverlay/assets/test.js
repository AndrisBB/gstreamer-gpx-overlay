var document = new Document("../src/plugins/gpxoverlay/assets/atom.svg");
this.Document = document;

var counter = 0;

function start(segment)
{
}

function render(point)
{
    // print("JS render called");

    counter = counter + 1;

    if(point != undefined) {
        // print(point.elevation);
        
        var el = Document.getElementById("timestamp_id");
        // if(el != undefined) {
        //     el.innerHTML = point.timestamp;
        // }
        if(el != undefined) {
            el.innerHTML = point.timestamp;
        }


        // el = Document.getElementById("hr_id");
        // if(el != undefined) {
        //     el.innerHTML = point.hr.toString() + " BPM";
        // }

        // el = Document.getElementById("heart_id");
        // if(el != undefined) {
        //     var fill = "#ffffff"
        //     if(counter >= 0 && counter < 15) {
        //         fill = "#ff0000";
        //     }
            
        //     if(counter > 30) {
        //         counter = 0;
        //     }
        //     el.setAttribute("fill", fill);
        // }
    }

    return document.stringify();
}