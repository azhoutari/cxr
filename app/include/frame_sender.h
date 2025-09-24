#pragma once
#include <QScreen>
#include <QApplication>
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <overlay_window.h>


class FrameSender {
public:
    FrameSender(QScreen* screen, const std::string& url, OverlayWindow* window);
    ~FrameSender();

    void captureAndSendLoop(int fps = 5); // starts capturing
    void stop();                          // stops the loop

private:
    QScreen* screen_;
    OverlayWindow* overlayWindow_;

    std::string url_;
    CURL* curl_;
    bool running_;
    std::thread senderThread_;

    // Helper functions
    cv::Mat QImageToCvMat(const QImage& image);
    std::string sendFrame(const cv::Mat& frame);
};
