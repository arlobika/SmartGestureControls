/**
 * HandTracker.hpp
 * 
 * Manages communication with a Python subprocess that runs MediaPipe hand detection.
 * 
 * Communication Protocol:
 * - C++ encodes frames to JPEG, then base64
 * - Sends size + base64 data to Python via pipe (stdin)
 * - Python returns JSON with hand landmarks via pipe (stdout)
 * - C++ parses JSON to extract hand data
 */

#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

/**
 * Represents a single detected hand with its landmarks
 */
struct Hand {
    std::vector<cv::Point> landmarks;  // 21 hand landmarks (pixel coordinates)
    float confidence;                  // Detection confidence score (0.0 - 1.0)
    std::string handedness;           // "Left" or "Right"
};

/**
 * HandTracker Class
 * 
 * Manages a Python subprocess that performs hand detection using MediaPipe.
 * Handles frame encoding, communication, and JSON parsing.
 */
class HandTracker {
public:
    /**
     * Constructor: Starts Python subprocess with MediaPipe
     * - Launches hand_detector.py
     * - Waits for "READY" signal from Python
     * - Sets initialized_ flag on success
     */
    HandTracker();
    
    /**
     * Destructor: Cleanup Python subprocess
     * - Closes pipe to Python process
     * - Terminates subprocess
     */
    ~HandTracker();
    
    /**
     * Detect hands in the given frame
     * 
     * @param frame OpenCV Mat containing the image to process
     * @return Vector of Hand objects (0-2 hands typically)
     * 
     * Process:
     * 1. Encode frame to JPEG
     * 2. Encode JPEG to base64 (for text pipe compatibility)
     * 3. Send size + base64 data to Python
     * 4. Read JSON response from Python
     * 5. Parse JSON to extract hand landmarks
     * 6. Return Hand objects
     */
    std::vector<Hand> detectHands(const cv::Mat& frame);
    
    /**
     * Check if HandTracker was initialized successfully
     * @return true if Python subprocess is running
     */
    bool isInitialized() const { return initialized_; }
    
private:
    /**
     * Start the Python subprocess
     * @return true if successful, false otherwise
     */
    bool startPythonProcess();
    
    /**
     * Stop and cleanup the Python subprocess
     */
    void stopPythonProcess();
    
    bool initialized_;      // Flag indicating successful initialization
    FILE* python_process_;  // Bidirectional pipe to Python subprocess
};


