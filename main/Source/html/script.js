
let streamUrl = document.location.origin + ':81' + "/stream"
let serverUrl = document.location.origin;

//let streamUrl = "http://192.168.1.204:81/stream"
//let serverUrl = "http://192.168.1.204"


let streamToggle = 0;
let faceDetectToggle = 0; //0 off 1 multipipe

document.addEventListener('DOMContentLoaded', function (event) {

    el = document.getElementById("Toggle stream");
    el.addEventListener("click", ToggleStream);
    el.addEventListener("click", UpdateConfig);

    el = document.getElementById("selectNN");
    el.addEventListener("change", ToggleFaceDetect);
    el.addEventListener("change", UpdateConfig);

    el = document.getElementById("selectRES");
    el.addEventListener("change", UpdateConfig);

    el = document.getElementById("Toggle face detect");
    el.addEventListener("click", ToggleFaceDetect);
    el.addEventListener("click", UpdateConfig);

    /*Toggle stream*/

    function ToggleStream() {
        const streamView = document.getElementById('stream')
        streamButton = document.getElementById("Toggle stream");
        streamContainer = document.getElementById("stream-container");
        if (streamToggle == 0) {
            streamToggle = 1;
            streamButton.textContent = "Stop stream"
            streamContainer.style.height = "auto"
            streamView.style.visibility = "visible"
            setTimeout(StreamEnable, 1000);
        } else {
            streamToggle = 0;
            streamButton.textContent = "Start stream"
            streamContainer.style.height = "100px"
            streamView.style.visibility = "hidden"
        }
    }


    function ToggleFaceDetect() {
        toggleButton = document.getElementById("Toggle face detect");
        neuralNetwork = document.getElementById("selectNN");
        if (faceDetectToggle == 0) {
            faceDetectToggle = neuralNetwork.value;
            toggleButton.textContent = "Stop Face Detect"
        } else {
            faceDetectToggle = 0;
            toggleButton.textContent = "Start Face Detect"
        }
    }



    function UpdateConfig() {
        paramUrl = serverUrl + "/control?"
        let update = 0;
        el = this;
        if (el.className === "inputSelect") {
            paramValue = el.value;
            paramName = el.name;
            update = 1;
        } else if (el.className === "button") {
            paramName = el.name;
            switch (paramName) {
                case "EnableStream": { paramValue = streamToggle; update = 1; break; }
                case "FaceDetect": { paramValue = faceDetectToggle; update = 1; break; }
                default: paramValue = 0;
            }
        }


        if (update) {
            paramUrl += "var=" + paramName + ";value=" + paramValue;
            fetch(paramUrl);
        }
    }

    statusURl = serverUrl + "/status";
    fetch(statusURl)
        .then(function (response) {
            return response.json()
        })
        .then(function (responseJson) {
            value = responseJson.NeuralNetwork;
            el = document.getElementsByName("NeuralNetwork")[0];
            el.value = value;
            value = responseJson.Resolution;
            el = document.getElementsByName("Resolution")[0];
            el.value = value;
        })
})


/* Use Canvas to draw every frame and draw face rectangle */

const canvas = document.getElementById('stream');
const ctx = canvas.getContext('2d');

function StreamEnable() {

    fetch(streamUrl)
        .then(response => {
            const reader = response.body.getReader();
            let buffer = new Uint8Array(0);

            function readChunk() {
                reader.read().then(({ done, value }) => {
                    if (done) {
                        console.log('Stream ended');
                        return;
                    }

                    // Merge new chunk with existing buffer
                    const newBuffer = new Uint8Array(buffer.length + value.length);
                    newBuffer.set(buffer);
                    newBuffer.set(value, buffer.length);
                    buffer = newBuffer;

                    let start, end;
                    while ((start = findStart(buffer)) !== -1 && (end = findEnd(buffer, start)) !== -1) {
                        const jpeg = buffer.slice(start, end + 1);
                        buffer = buffer.slice(end + 1); // remove processed JPEG

                        // Decode JPEG directly to ImageBitmap (no blob, no URL)
                        createImageBitmap(new Blob([jpeg], { type: 'image/jpeg' }))
                            .then(bitmap => {
                                canvas.width = bitmap.width;
                                canvas.height = bitmap.height;
                                ctx.drawImage(bitmap, 0, 0);
                                if (faceDetectToggle == 1) //Vyksta detectas
                                    detectFaces();
                            })
                            .catch(console.error);
                    }

                    readChunk(); // Continue reading
                });
            }
            if (streamToggle == 1)
                readChunk();
        });
}


/* Functions for parsing  JPEG and drawing on canvas*/

function findStart(buffer) {
    for (let i = 0; i < buffer.length - 1; i++) {
        if (buffer[i] === 0xff && buffer[i + 1] === 0xd8) return i;
    }
    return -1;
}

function findEnd(buffer, startIndex = 0) {
    for (let i = startIndex; i < buffer.length - 1; i++) {
        if (buffer[i] === 0xff && buffer[i + 1] === 0xd9) return i + 1;
    }
    return -1;
}
