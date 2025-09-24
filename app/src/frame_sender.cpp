#include "frame_sender.h"
#include "detections.h"
#include <chrono>
#include <iostream>
#include <QMetaObject> // for invoking methods on the main thread


// This function will be called by libcurl to write the response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

FrameSender::FrameSender(QScreen* screen, const std::string& url, OverlayWindow* window)
    : screen_(screen), url_(url), running_(true), overlayWindow_(window)
{
    curl_global_init(CURL_GLOBAL_ALL);
    curl_ = curl_easy_init();
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

FrameSender::~FrameSender() {
    stop();
    if (curl_) curl_easy_cleanup(curl_);
    curl_global_cleanup();
}

cv::Mat FrameSender::QImageToCvMat(const QImage& image) {
    QImage conv = image.convertToFormat(QImage::Format_RGB888);
    cv::Mat mat(conv.height(), conv.width(), CV_8UC3,
                const_cast<uchar*>(conv.bits()),
                conv.bytesPerLine());
    cv::Mat mat_bgr;
    cv::cvtColor(mat, mat_bgr, cv::COLOR_RGB2BGR);
    return mat_bgr;
}

std::string FrameSender::sendFrame(const cv::Mat& frame) {
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);

    std::string response_string;


    curl_mime* form = curl_mime_init(curl_);
    curl_mimepart* field = curl_mime_addpart(form);
    curl_mime_name(field, "frame");
    curl_mime_filename(field, "frame.jpg");
    curl_mime_data(field, reinterpret_cast<const char*>(buf.data()), buf.size());
    curl_mime_type(field, "image/jpeg");

    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, 5000L); // handle server delay

    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_string);


    CURLcode res = curl_easy_perform(curl_);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;

        curl_mime_free(form);
        
        return "";
    } else {
        curl_mime_free(form);

        return response_string;
    }

    

    curl_mime_free(form);
}

void FrameSender::captureAndSendLoop(int fps) {
    senderThread_ = std::thread([this, fps]() {
        while (running_) {
            QPixmap pixmap = screen_->grabWindow(0);
            QImage qimg = pixmap.toImage();
            cv::Mat frame = QImageToCvMat(qimg);

            std::string jsonStr = sendFrame(frame); // implement your curl request

            // Parse detections
            auto detections = parseDetections(jsonStr);

            if (overlayWindow_) {
                // Use QMetaObject::invokeMethod to ensure the call is thread-safe
                QMetaObject::invokeMethod(overlayWindow_, "setDetections", Qt::QueuedConnection, Q_ARG(std::vector<Detection>, detections));
            }

            // PRint detections for debugging
            for (const auto& det : detections) {
                std::cout << "Class: " << det.label 
                          << " Confidence: " << det.confidence 
                          << " Box: [" << det.x1 << ", " << det.y1 
                          << ", " << det.x2 << ", " << det.y2 << "]" 
                          << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
        }
    });
}

void FrameSender::stop() {
    running_ = false;
    if (senderThread_.joinable())
        senderThread_.join();
}