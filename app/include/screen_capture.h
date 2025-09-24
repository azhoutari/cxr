#pragma once
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QImage>

class ScreenCapture {
public:
    ScreenCapture(QScreen* screen);

    QImage grabRegion(int x, int y, int width, int height);
    QImage grabFull();

private:
    QScreen* screen;
};
