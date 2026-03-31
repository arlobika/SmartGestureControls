/**
 * @file HandTracker.hpp
 * @brief Declares the HandTracker class used to detect hands using MediaPipe via a Python subprocess.
 *
 * Manages communication with a Python subprocess that runs MediaPipe hand detection.
 * 
 * Communication Protocol:
 * - C++ encodes frames to JPEG, then base64
 * - Sends size + base64 data to Python via pipe (stdin)
 * - Python returns JSON with hand landmarks via pipe (stdout)
 * - C++ parses JSON to extract hand data
 * @authors Hasit
 */

#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

/**
 * @struct Hand
 * @brief Represents a single detected hand with its landmarks
 * Each detected hand contains a set of 21 landmarks returned by MediaPipe.
 * @authors
 */
struct Hand {
    std::vector<cv::Point> landmarks;  // 21 hand landmarks (pixel coordinates)
    float confidence;                  // Detection confidence score (0.0 - 1.0)
    std::string handedness;           // "Left" or "Right"
};

/**
 * @class HandTracker
 * @brief Manages a Python subprocess that performs hand detection using MediaPipe.
 * Handles frame encoding, communication, and JSON parsing.
 * @authors
 */
class HandTracker {
public:
    /**
     * @class Handtracker
     * @brief Starts Python subprocess with MediaPipe
     * - Launches hand_detector.py
     * - Waits for "READY" signal from Python
     * - Sets initialized_ flag on success
     * @authors Hasit
     */
    HandTracker();
    
    /**
     * @class ~HandTracker
     * @brief Deconstructor that cleansup Python subprocess
     * - Closes pipe to Python process
     * - Terminates subprocess
     * @authors Hasit
     */
    ~HandTracker();
    
    /**
     * @brief Detect hands in the given frame
     * This function sends the given frame to the Python MediaPipe detector
     * and receives detection results in JSON format. The JSON response is
     * parsed to extract landmark coordinates detected hand.
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
     * @authors
     */
    std::vector<Hand> detectHands(const cv::Mat& frame);
    
    /**
     * @brief Check if HandTracker was initialized successfully
     * @return true if Python subprocess is running
     * @authors
     */
    bool isInitialized() const { return initialized_; }
    
private:
    /**
     * @brief Launches the Python MediaPipe detection subprocess.
     * This function starts the Python script responsible for performing
     * hand detection and establishes a bidirectional communication pipe
     * between the C++ program and the Python process.
     * @return true if successful, false otherwise
     * @authors
     */
    bool startPythonProcess();
    
    /**
     * @brief Stop and cleanup the Python subprocess
     * This function closes the communication pipe and terminates the
     * Python detection process to ensure that system resources are
     * properly released.
     * @authors Hasit
     */
    void stopPythonProcess();
    
    bool initialized_;      // Flag indicating successful initialization
    FILE* python_process_;  // Bidirectional pipe to Python subprocess
};


