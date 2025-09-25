#include "detections.h"

// Parse JSON
std::vector<Detection> parseDetections(const std::string& jsonStr, qreal devicePixelRatio) {
    std::vector<Detection> detections;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(jsonStr).toUtf8());
    if (!doc.isObject()) return detections;

    QJsonObject obj = doc.object();
    QJsonArray arr = obj["detections"].toArray();

    for (auto item : arr) {
        QJsonObject det = item.toObject();
        Detection d;
        d.x1 = det["x1"].toInt() / devicePixelRatio;
        d.y1 = det["y1"].toInt() / devicePixelRatio;
        d.x2 = det["x2"].toInt() / devicePixelRatio;
        d.y2 = det["y2"].toInt() / devicePixelRatio;
        d.label = det["label"].toString().toStdString();
        d.confidence = det["confidence"].toDouble();
        detections.push_back(d);
    }
    return detections;
}

// Draw rectangles and labels
void drawDetections(cv::Mat& frame, const std::vector<Detection>& detections) {
    for (const auto& d : detections) {
        cv::rectangle(frame, cv::Point(d.x1, d.y1), cv::Point(d.x2, d.y2),
                      cv::Scalar(0, 255, 0), 2);
        std::string text = d.label + " " + std::to_string(int(d.confidence * 100)) + "%";
        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.6, 1, &baseline);
        cv::rectangle(frame, cv::Point(d.x1, d.y1 - textSize.height - 5),
                      cv::Point(d.x1 + textSize.width, d.y1),
                      cv::Scalar(0, 255, 0), cv::FILLED);
        cv::putText(frame, text, cv::Point(d.x1, d.y1 - 5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,0), 1);
    }
}

// Convert cv::Mat → QImage
QImage matToQImage(const cv::Mat& mat) {
    cv::Mat rgb;
    if (mat.channels() == 3)
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
    else
        rgb = mat.clone();
    return QImage((const uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
}
