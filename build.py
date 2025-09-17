import os
import subprocess

if __name__ == '__main__':
    is_windows = os.name == 'nt' 

    if is_windows:
        subprocess.call(['pyinstaller', '--windowed', '--onedir', '--name', 'main', '--add-data', 'models/best.onnx;models', '--hidden-import', 'numpy', '--hidden-import', 'cv2', 'main.py'])
    else:
        subprocess.call(['pyinstaller', '--windowed', '--onedir', '--name', 'main', '--add-data', 'models/best.onnx:models', '--hidden-import', 'numpy', '--hidden-import', 'cv2', 'main.py'])

