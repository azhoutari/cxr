import sys, os
import re
import time
import threading
from threading import Lock
from dataclasses import dataclass, field
from typing import List, Tuple, Dict

import numpy as np
import mss
import cv2

from PyQt5.QtCore import Qt, QRect, QTimer, pyqtSignal, QObject
from PyQt5.QtGui import QPainter, QColor, QPen, QFont, QGuiApplication
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QShortcut
from PyQt5.QtGui import QKeySequence

# ---- Windows-only window binding (fallback to full screen if not found)
try:
    import win32gui
    import win32con
except ImportError:
    win32gui = None

# # ===================== CONFIG =====================
# WINDOW_TITLE_REGEX = r"(OHIF|Radiant|PACS|Viewer|Carestream|GE|Philips|Agfa|Sectra)"
# CONF_THRESHOLD_SHOW = 0.40   # box appears above this
# CONF_THRESHOLD_HIDE = 0.30   # stays visible if previous frame had it and conf above this
# IOU_THRESHOLD = 0.45
# TARGET_FPS = 10              # limit processing rate
# FONT_FAMILY = "Arial"
# FONT_SIZE = 14
# BOX_THICKNESS = 2
# MODEL_PATH = r"C:\Users\malza\Downloads\FinalFinal4\FinalFinal4\weights\best.pt"
# # ==================================================

# ===================== CONFIG =====================
WINDOW_TITLE_REGEX = r"(OHIF|Radiant|PACS|Viewer|Carestream|GE|Philips|Agfa|Sectra)"
CONF_THRESHOLD_SHOW = 0.40
CONF_THRESHOLD_HIDE = 0.30
IOU_THRESHOLD = 0.45
TARGET_FPS = 10
FONT_FAMILY = "Arial"
FONT_SIZE = 14
BOX_THICKNESS = 2

# Recommended when running on CPU
#IMGSZ = 1280   # try 1536 if PCs are strong; 1024 if weak
# ==================================================

def resource_path(rel_path):
    """ Get absolute path to resource (works for PyInstaller onefile) """
    if hasattr(sys, "_MEIPASS"):
        return os.path.join(sys._MEIPASS, rel_path)
    return os.path.join(os.path.abspath("."), rel_path)

MODEL_PATH = resource_path("models/best.onnx")

@dataclass
class Detection:
    x1: int
    y1: int
    x2: int
    y2: int
    label: str
    conf: float

    def center(self) -> Tuple[int, int]:
        return (int((self.x1 + self.x2) / 2), int((self.y1 + self.y2) / 2))
    


class InferenceEngine:
    """Real inference engine using Ultralytics (ONNX backend on CPU)."""
    def __init__(self, model_path: str):
        from ultralytics import YOLO
        self.model = YOLO(model_path)
        # Warmup so the first real frame isn't slow
        self.model.predict(
            np.zeros((256, 256, 3), dtype=np.uint8),
            verbose=False, conf=0.01, iou=0.01, imgsz=256
        )

    def detect(self, frame_bgra: np.ndarray) -> List[Detection]:
        frame_rgb = cv2.cvtColor(frame_bgra, cv2.COLOR_BGRA2RGB)

        # Ultralytics will route this to ONNX Runtime since MODEL_PATH is .onnx
        results = self.model.predict(
            frame_rgb,
            verbose=False,
            conf=CONF_THRESHOLD_HIDE,   # slightly lower; hysteresis handles flicker
            iou=IOU_THRESHOLD,
            #imgsz=IMGSZ,                # <-- key for CPU speed
            max_det=50
        )

        dets: List[Detection] = []
        if not results:
            return dets

        r0 = results[0]
        names = r0.names if hasattr(r0, "names") else getattr(self.model, "names", None)

        if getattr(r0, "boxes", None) is None:
            return dets

        for b in r0.boxes:
            x1, y1, x2, y2 = map(int, b.xyxy[0].tolist())
            conf = float(b.conf[0])
            cls = int(b.cls[0])
            label = names.get(cls, str(cls)) if isinstance(names, dict) else (names[cls] if names else str(cls))
            dets.append(Detection(x1, y1, x2, y2, label, conf))
        return dets


class HysteresisFilter:
    """
    Simple temporal hysteresis to reduce flicker:
    - Show if conf >= show_thresh,
    - Keep showing next frame if conf >= hide_thresh and it matches a previous box by center proximity.
    """
    def __init__(self, show_thresh: float, hide_thresh: float, max_dist: int = 30):
        self.show_thresh = show_thresh
        self.hide_thresh = hide_thresh
        self.max_dist = max_dist
        self.prev: List[Detection] = []

    def _match(self, d: Detection, prev_list: List[Detection]) -> bool:
        cx, cy = d.center()
        for p in prev_list:
            px, py = p.center()
            if abs(cx - px) <= self.max_dist and abs(cy - py) <= self.max_dist and d.label == p.label:
                return True
        return False

    def apply(self, dets: List[Detection]) -> List[Detection]:
        keep: List[Detection] = []
        for d in dets:
            if d.conf >= self.show_thresh:
                keep.append(d)
            elif d.conf >= self.hide_thresh and self._match(d, self.prev):
                keep.append(d)
        self.prev = keep
        return keep


class OverlayScreen(QWidget):
    def __init__(self):
        super().__init__()

        # Screen/overlay
        self.screen = QGuiApplication.primaryScreen()
        self.setFixedSize(self.screen.size())
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        self.setAttribute(Qt.WA_TranslucentBackground)
        
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint)

        # States
        self.running = True
        self.infer_active = False
        self.hidden = False
        self._detections: List[Detection] = []
        self._lock = Lock()
        self._window_rect = self._find_viewer_rect()  # QRect or full screen fallback

        # Inference & capture
        self.engine = InferenceEngine(MODEL_PATH)
        self.hyst = HysteresisFilter(CONF_THRESHOLD_SHOW, CONF_THRESHOLD_HIDE)

        # Timer for repaint
        self.timer = QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(int(1000 / max(1, TARGET_FPS * 2)))  # repaint faster than infer

        # Background thread for capture+infer
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

        # Keyboard shortcuts
        QShortcut(QKeySequence("S"), self, activated=self.toggle_inference)
        QShortcut(QKeySequence("H"), self, activated=self.toggle_hidden)
        QShortcut(QKeySequence("Esc"), self, activated=self.close_overlay)

        # UI
        self.rectangles = []

    # ---------- Window binding ----------
    def _find_viewer_rect(self) -> QRect:
        if win32gui is None:
            # Fallback: full screen capture
            return QRect(0, 0, self.width(), self.height())

        target = None
        def enum_handler(hwnd, _):
            nonlocal target
            if win32gui.IsWindowVisible(hwnd):
                title = win32gui.GetWindowText(hwnd)
                if title and re.search(WINDOW_TITLE_REGEX, title, flags=re.IGNORECASE):
                    # prefer the first matching foreground-like window
                    target = hwnd

        win32gui.EnumWindows(enum_handler, None)
        if target:
            left, top, right, bottom = win32gui.GetWindowRect(target)
            # clamp to screen bounds
            left = max(0, left); top = max(0, top)
            right = min(self.width(), right); bottom = min(self.height(), bottom)
            if right > left and bottom > top:
                return QRect(left, top, right - left, bottom - top)

        # Fallback
        return QRect(0, 0, self.width(), self.height())

    # ---------- Control ----------
    def toggle_inference(self):
        self.infer_active = not self.infer_active

    def toggle_hidden(self):
        self.hidden = not self.hidden
        self.update()

    def close_overlay(self):
        self.running = False
        QApplication.quit()
        sys.exit(0)

    # ---------- Capture + Inference Loop ----------
    def _loop(self):
        sct = mss.mss()
        last = 0.0
        target_dt = 1.0 / max(1, TARGET_FPS)

        # Monitor region
        rect = self._window_rect
        monitor = {"top": rect.y(), "left": rect.x(), "width": rect.width(), "height": rect.height()}

        while self.running:
            start = time.time()
            if self.infer_active and not self.hidden:
                try:
                    frame = np.array(sct.grab(monitor))  # BGRA
                    dets = self.engine.detect(frame)
                    dets = self.hyst.apply(dets)
                    with self._lock:
                        # shift boxes to overlay coords (already global)
                        # We draw using absolute screen coords, so no shift needed for top-level overlay
                        self._detections = dets
                except Exception as e:
                    # Never crash; just skip the frame
                    print(f"[WARN] Inference error: {e}")

            # Frame pacing
            elapsed = time.time() - start
            sleep_for = max(0.0, target_dt - elapsed)
            time.sleep(sleep_for)

    # ---------- Painting ----------
    def paintEvent(self, event):
        if self.hidden:
            return

        painter = QPainter(self)
        painter.setCompositionMode(QPainter.CompositionMode_Clear)
        painter.fillRect(self.rect(), QColor(0, 0, 0, 0))
        painter.setCompositionMode(QPainter.CompositionMode_SourceOver)

        painter.setPen(QPen(Qt.red, BOX_THICKNESS))
        painter.setFont(QFont(FONT_FAMILY, FONT_SIZE))

        with self._lock:
            dets = list(self._detections)

        # Clip drawing to the viewer region only (no boxes outside)
        rect = self._window_rect
        painter.setClipRect(rect)

        for d in dets:
            # Ensure rect within our capture window
            x1 = max(rect.left(), d.x1)
            y1 = max(rect.top(), d.y1)
            x2 = min(rect.right(), d.x2)
            y2 = min(rect.bottom(), d.y2)
            if x2 <= x1 or y2 <= y1:
                continue

            painter.drawRect(QRect(x1, y1, x2 - x1, y2 - y1))
            painter.drawText(x1 + 5, y1 - 6, f"{d.label} {d.conf*100:.1f}%")

        painter.end()


class ControlPanel(QWidget):
    def __init__(self, overlay: OverlayScreen):
        super().__init__()
        self.overlay = overlay
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool)
        self.setFixedSize(260, 40)

        self.btn_exit = QPushButton("Exit (Esc)", self)
        self.btn_exit.setGeometry(0, 5, 110, 30)
        self.btn_exit.clicked.connect(self.overlay.close_overlay)

        self.btn_start = QPushButton("Start (S)", self)
        self.btn_start.setGeometry(120, 5, 70, 30)
        self.btn_start.clicked.connect(self._toggle_inference)

        self.btn_hide = QPushButton("Hide (H)", self)
        self.btn_hide.setGeometry(200, 5, 60, 30)
        self.btn_hide.clicked.connect(self.overlay.toggle_hidden)

        # drag variables
        self.dragging = False
        self.offset = None

        self.show()

    def _toggle_inference(self):
        self.overlay.toggle_inference()
        self.btn_start.setText("Pause (S)" if self.overlay.infer_active else "Start (S)")

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.dragging = True
            self.offset = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()

    def mouseMoveEvent(self, event):
        if self.dragging:
            new_pos = event.globalPos() - self.offset
            self.move(new_pos)
            event.accept()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.dragging = False
            event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)

    overlay = OverlayScreen()
    overlay.show()

    controls = ControlPanel(overlay)
    sys.exit(app.exec())