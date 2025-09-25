#include "overlay_window.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication> 
#include <QPainter>
#include <QDebug>
#include <QKeySequence> // Required for QShortcut key definition


OverlayWindow::OverlayWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window |
                   Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::Tool);

    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Set the geometry to cover the full screen
    if (QScreen* screen = QApplication::primaryScreen()) {
        setGeometry(screen->geometry());
    }

    visibilityShortcut = new QShortcut(QKeySequence(Qt::Key_H), this);
    visibilityShortcut->setContext(Qt::ApplicationShortcut); // <-- CRITICAL FIX

    connect(visibilityShortcut, &QShortcut::activated, this, &OverlayWindow::toggleVisibility);


}

void OverlayWindow::setDetections(const std::vector<Detection>& detections) {
    detections_ = detections;
    qDebug() << "Received detections, scheduling repaint";
    update(); // This schedules a repaint, which calls paintEvent()
}

void OverlayWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (auto &detection : detections_) {
        QRect box(detection.x1, detection.y1, detection.x2 - detection.x1, detection.y2 - detection.y1);
        
        QPen pen(Qt::green, 2);
        painter.setPen(pen);
        
        // Semi-transparent fill
        painter.setBrush(QBrush(QColor(0, 255, 0, 50))); 
        
        painter.drawRect(box);
        
        QString label = QString("%1: %2%").arg(QString::fromStdString(detection.label)).arg(int(detection.confidence * 100));
        painter.drawText(box.topLeft() + QPoint(5, 15), label);
    }
}

void OverlayWindow::toggleVisibility()
{
    if (this->isVisible()) {
        this->hide();
        qDebug() << "Overlay Hidden (Shortcut triggered)";
    } else {
        // When unhiding, ensure it is raised back to the top
        this->show();
        this->raise(); 
        this->activateWindow();
        qDebug() << "Overlay Shown (Shortcut triggered)";
    }
}