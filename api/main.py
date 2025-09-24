from ultralytics import YOLO

from typing import Union

from fastapi import FastAPI, File, UploadFile
import numpy as np
import cv2

model = YOLO('../models/best.onnx', verbose=False)

app = FastAPI()

@app.get("/")
def read_root():
    return {"Hello": "World"}


@app.post("/infer/")
async def infer_frame(frame: UploadFile = File(...)):
    # Read raw bytes
    contents = await frame.read()

    # Convert bytes → NumPy array
    nparr = np.frombuffer(contents, np.uint8)

    # Decode JPEG → OpenCV image (BGR)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    result = model.predict(img)

    detections = []

    if not result:
        return {"detections": detections}
    
    names = result[0].names if hasattr(result[0], "names") else getattr(model, "names", None)

    if getattr(result[0], "boxes", None) is None:
        return {"detections": detections}
    

    # For testing purposes
    detections.append({
        'x1': 300,
        'y1': 300,
        'x2': 400,
        'y2': 400,
        'label': 'cat',
        'confidence': 0.9
    })
    
    for b in result[0].boxes:
        x1, y1, x2, y2 = map(int, b.xyxy[0].tolist())
        conf = float(b.conf[0])
        cls = int(b.cls[0])
        label = names.get(cls, str(cls)) if isinstance(names, dict) else (names[cls] if names else str(cls))
        detections.append({
            "x1": x1,
            "y1": y1,
            "x2": x2,
            "y2": y2,
            "label": label,
            "confidence": conf
        })


    return {"detections": detections}
