#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <QLabel>
#include <QPixmap>
#include <QImage>

struct Detection {
    int x1, y1, x2, y2;
    std::string label;
    float confidence;
};

// Parse server JSON response
std::vector<Detection> parseDetections(const std::string& jsonStr);

// Draw detections on a frame
void drawDetections(cv::Mat& frame, const std::vector<Detection>& detections);

// Convert cv::Mat to QImage for Qt display
QImage matToQImage(const cv::Mat& mat);
