# CXR Overlay Demo

## Last Updated: 15/9/2025
## Author: Abdulaziz Houtari

### Environment:
- Python 3.x
- Numpy (version 1.xx)

### Setup Instructions
1. Clone the repository:
   ```bash
   git clone <repository-url>
   cd <repository-folder>
   ```

2. Install the required dependencies
    ```bash
    pip install -r requirements.txt
    ```

3. Place the ONNX model file in the **models**
    ```bash
    models/best.onnx
    ```

4. Run the following script
    ```
    python build.py
    ```

5. It will then be under `dist/CXR.exe` or `dist/CXR.app`