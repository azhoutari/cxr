# Detection Overlay App

This project consists of a C++ desktop application and a Python FastAPI backend for real-time object detection. The C++ application captures the screen, sends frames to the FastAPI API for inference, and receives detection results to display as an overlay.

-----

### Project Structure

```
.
├── /app
│   ├── /src
│   ├── /include
|   |── /build 
│   ├── CMakeLists.txt
│   └── build.sh
├── /api
│   └── main.py
├── /models
│   └── best.onnx
├── requirements.txt
└── README.md
```

-----

### Prerequisites

#### C++ Application

The C++ application requires the following libraries:

  * **Qt6**: Used for the GUI, window management, and display.
  * **OpenCV**: Used for image processing, including converting frames to the correct format before sending.
  * **CURL**: Used for sending HTTP requests to the FastAPI backend.

Make sure these libraries are installed and accessible by your build system. The provided `CMakeLists.txt` is expected to find these dependencies automatically.

#### FastAPI Backend (For testing purposes)

The backend requires the Python libraries listed in `requirements.txt`. You can install them using pip:

```bash
pip install -r requirements.txt
```

-----

### Building and Running

#### C++ Application

To build the C++ application, you can use the `build.sh` script provided. This script is designed for Linux and macOS environments.

Make sure to give correct permissions:

```bash
chmod +x ./build.sh
```

```bash
./build.sh
```

To run CMake again before building (for example, after changing CMakeLists.txt), you can use the rebuild argument:

```bash
./build.sh rebuild
```

Alternatively, you can build using CMake manually:

1.  Create and navigate to the build directory.
    ```bash
    cd app/build
    ```
2.  Run CMake to configure the project.
    ```bash
    cmake ..
    ```
3.  Build the project.
    ```bash
    cmake --build .
    ```

The executable will be generated in the `app/build` directory.

#### FastAPI Backend

To run the backend API, navigate to the `/api` directory and run the `main.py` file with Faatapi. The application uses the `best.onnx` model from the `/models` directory.

```bash
cd api
fastapi dev main.py
```

### API Documentation

The API provides a single endpoint for receiving a video frame and returning object detection results.

  * **Endpoint:** `/infer/`
  * **Method:** `POST`
  * **Description:** This endpoint takes a single image file (the picture/video frame) as input and returns a JSON array of detections.
  * **Request Body:**
      * `frame`: `UploadFile` (image file)
  * **Response:**
      * `200 OK`:
      
        ```json
        {
          "detections": [
            {
              "x1": 300,
              "y1": 300,
              "x2": 400,
              "y2": 400,
              "label": "xxx",
              "confidence": 0.9
            },
            ...
          ]
        }
        ```