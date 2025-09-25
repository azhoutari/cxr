#pragma once
#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QShortcut> // <-- New Include
#include <detections.h>

class OverlayWindow : public QWidget {
    Q_OBJECT

public:
    OverlayWindow(QWidget* parent = nullptr);

public slots:
    void setDetections(const std::vector<Detection>& detections); // New function to update detections

private slots:
    void toggleVisibility(); // Slot to toggle visibility

protected:
    void paintEvent(QPaintEvent* event) override; // Override paintEvent

private:
    std::vector<Detection> detections_;
    QShortcut* visibilityShortcut; // Shortcut to toggle visibility


};
