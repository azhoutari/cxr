#include "screen_capture.h"

ScreenCapture::ScreenCapture(QScreen* screen) : screen(screen) {}

QImage ScreenCapture::grabRegion(int x, int y, int width, int height) {
    return screen->grabWindow(0, x, y, width, height).toImage();
}

QImage ScreenCapture::grabFull() {
    return screen->grabWindow(0).toImage();
}
