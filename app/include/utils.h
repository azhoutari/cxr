#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>

inline cv::Mat QImageToCvMat(const QImage& image) {
    QImage conv = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(conv.height(), conv.width(), CV_8UC3,
                const_cast<uchar*>(conv.bits()),
                conv.bytesPerLine());
    cv::Mat mat_bgr;
    cv::cvtColor(mat, mat_bgr, cv::COLOR_RGB2BGR);
    return mat_bgr;
}
