#include <QApplication>
#include "frame_sender.h"
#include "screen_capture.h"
#include "overlay_window.h"
#include "utils.h"

#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return -1;

    OverlayWindow window;
    window.show();

    FrameSender sender(screen, "http://127.0.0.1:8000/infer/", &window);
    sender.captureAndSendLoop(5); // 5 fps



    return app.exec(); // Qt event loop keeps app alive
}