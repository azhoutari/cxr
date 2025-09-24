#pragma once
#include <QWidget>
#include <QPoint>
#include <detections.h>

class OverlayWindow : public QWidget {
    Q_OBJECT

public:
    OverlayWindow(QWidget* parent = nullptr);

public slots:
    void setDetections(const std::vector<Detection>& detections); // New function to update detections

protected:
    void paintEvent(QPaintEvent* event) override; // Override paintEvent
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPoint dragPosition;
    std::vector<Detection> detections_;

};
