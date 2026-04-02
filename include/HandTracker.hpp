#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Pull in the minimum platform headers needed for member variable types
#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX          // prevent Windows.h from defining min/max macros
#    define NOMINMAX        // which would break std::min / std::max everywhere
#  endif
#  include <Windows.h>   // HANDLE, INVALID_HANDLE_VALUE
#else
#  include <sys/types.h> // pid_t
#endif

// ── Hand ─────────────────────────────────────────────────────────────────────
// Holds the result for a single detected hand.
struct Hand {
    std::string            handedness; // "Left" or "Right"
    float                  confidence; // MediaPipe handedness confidence [0, 1]
    std::vector<cv::Point> landmarks;  // 21 pixel-space landmark positions
};

// ── HandTracker ───────────────────────────────────────────────────────────────
// Manages a long-lived Python subprocess (hand_detector.py) and communicates
// with it over two anonymous pipes — one for writing frames (stdin of the child)
// and one for reading JSON results (stdout of the child).
//
// Usage:
//   HandTracker tracker;
//   if (!tracker.isInitialized()) { /* handle error */ }
//   std::vector<Hand> hands = tracker.detectHands(frame);
class HandTracker {
public:
    HandTracker();
    ~HandTracker();

    // Not copyable or movable — owns OS-level pipe handles / fds.
    HandTracker(const HandTracker&) = delete;
    HandTracker& operator=(const HandTracker&) = delete;

    // Detect hands in the given BGR frame.
    // Returns an empty vector when no hands are found or on error.
    std::vector<Hand> detectHands(const cv::Mat& frame);

    bool isInitialized() const { return initialized_; }

private:
    bool startPythonProcess();
    void stopPythonProcess();

    bool initialized_;

    // Two separate FILE* streams replace the old single popen FILE*.
    // python_write_ → child stdin  (we send base64-encoded JPEG frames)
    // python_read_  ← child stdout (we receive JSON hand data)
    FILE* python_read_;
    FILE* python_write_;

    // Platform-specific child-process handle used for clean shutdown.
#ifdef _WIN32
    HANDLE child_process_handle_;
#else
    pid_t  child_pid_;
#endif
};