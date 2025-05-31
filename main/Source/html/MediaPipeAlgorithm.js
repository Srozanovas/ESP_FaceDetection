/*Detector Configuration */
const model = faceDetection.SupportedModels.MediaPipeFaceDetector;
const detectorConfig = {
    runtime: 'mediapipe',
    solutionPath: 'https://cdn.jsdelivr.net/npm/@mediapipe/face_detection',
};

const CreateDetector = async () => {
    detector = await faceDetection.createDetector(model, detectorConfig)
}


/*Detect faces and draw them on canvas*/

const detectFaces = async () => {
    const estimationConfig = { flipHorizontal: false };
    const faces = await detector.estimateFaces(canvas, estimationConfig);
    ctx.drawImage(canvas, 0, 0, canvas.width, canvas.height);
    faces.forEach(pred => {
        ctx.beginPath();
        ctx.lineWidth = "4";
        ctx.strokeStyle = "Blue";
        ctx.rect(
            pred.box.xMin,
            pred.box.yMin,
            pred.box.width,
            pred.box.height,
        );
        ctx.stroke();
    });
}

CreateDetector();
