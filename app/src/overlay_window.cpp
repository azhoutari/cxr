#include "overlay_window.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication> 
#include <QPainter>
#include <QDebug>


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

       // Toolbar container with solid background
    QWidget* container = new QWidget(this);
    container->setStyleSheet("background-color: rgba(50,50,50,0.9); border-radius: 5px;");

    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(5,5,5,5);
    layout->setSpacing(10);

    // Example label at top
    QLabel* title = new QLabel("CXR");
    title->setStyleSheet("color: white; font-weight: bold; font-size: 16px;");
    layout->addWidget(title);

    // Exit button (terminates the whole application)
    QPushButton* exitBtn = new QPushButton("X");
    exitBtn->setStyleSheet("background-color: red; color: white; border-radius: 5px;");
    connect(exitBtn, &QPushButton::clicked, qApp, &QApplication::quit); // quit entire app
    layout->addWidget(exitBtn);

    layout->addStretch(); // push buttons to top

    container->setLayout(layout);
    container->setFixedWidth(50); // width of toolbar
    container->setFixedHeight(400); // height of toolbar
    setFixedSize(container->size());
}

void OverlayWindow::setDetections(const std::vector<Detection>& detections) {
    detections_ = detections;
    qDebug() << "Received detections, scheduling repaint";
    update(); // This schedules a repaint, which calls paintEvent()
}

void OverlayWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    
    // VERY IMPORTANT: Fill the background with a transparent color
    painter.fillRect(event->rect(), QColor(0, 0, 0, 255)); // 1% alpha
    
    // Now draw your detections on this transparent canvas
    if (!detections_.empty()) {
        QColor rectColor(0, 255, 0, 150); 
        QPen pen(Qt::green, 2, Qt::SolidLine, Qt::RoundCap);
        
        painter.setPen(pen);
        painter.setBrush(QBrush(rectColor));
        
        for (const auto& d : detections_) {
            QRect rect(d.x1, d.y1, d.x2 - d.x1, d.y2 - d.y1);
            painter.drawRect(rect);
            
            QString label = QString::fromStdString(d.label) + QString(" %1%").arg(d.confidence * 100);
            painter.setPen(Qt::white);
            painter.drawText(d.x1, d.y1 - 5, label);
        }
    }
}

void OverlayWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void OverlayWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}
