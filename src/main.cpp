#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap(0, cv::CAP_AVFOUNDATION);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam\n";
        return 1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::imshow("Smart Gesture Controls", frame);
        if (cv::waitKey(1) == 27) break; // ESC
    }

    return 0;
}
