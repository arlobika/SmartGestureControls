/**
 * @file HandTracker.hpp
 * @brief Declares the HandTracker class used to detect hands
 * using MediaPipe via a Python subprocess.
 * Manages communication with a Python subprocess that runs MediaPipe hand detection.
 * Communication Protocol:
 * - Sends size + base64 data to Python via pipe (stdin)
 * - Python returns JSON with hand landmarks via pipe (stdout)
 * - C++ parses JSON to extract hand data
 * @authors Hasit, Jaime
 */

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

/**
 * Represents a single detected hand with its landmarks
 * @struct Hand
 * @brief Represents a single detected hand with its landmarks
 * Each detected hand contains a set of 21 landmarks returned by MediaPipe.
 * @authors Jaime, Hasit
 */

struct Hand {
    std::string            handedness; // "Left" or "Right"
    float                  confidence; // MediaPipe handedness confidence [0, 1]
    std::vector<cv::Point> landmarks;  // 21 pixel-space landmark positions
};

/**
 * @class HandTracker
 * @brief Manages a Python subprocess that performs hand detection using MediaPipe.
 * Handles frame encoding, communication, and JSON parsing.
 * @authors Hasit
 */

class HandTracker {
public:
    /**
    * @brief Starts Python subprocess with MediaPipe
    * - Launches hand_detector.py
    * - Waits for "READY" signal from Python
    * - Sets initialized_ flag on success
    * @authors Hasit
    */
    HandTracker();
    /**
        * @brief Deconstructor that cleansup Python subprocess
        * Closes pipe to Python process
        * Terminates subprocess
        * @authors Hasit
        */

    ~HandTracker();

    /**
     * @brief Copy constructor is deleted to prevent copying.
     * HandTracker manages a Python subprocess and file pipe. Allowing the object
     * to be copied could result in multiple objects attempting to control or
     * close the same process, which would lead to undefined behavior.
     * @author Jaime
     */
    HandTracker(const HandTracker&) = delete;
    /**
     * @brief Copy assignment operator is deleted to prevent assignment.
     * Disallowing copy assignment ensures that only one HandTracker instance
     * manages the underlying Python subprocess and communication pipe
     * @author Jaime
 */
    HandTracker& operator=(const HandTracker&) = delete;

    /**
     * @brief Detect hands in the given frame
     * This function sends the given frame to the Python MediaPipe detector
     * and receives detection results in JSON format. The JSON response is
     * parsed to extract landmark coordinates detected hand.
     * @param frame OpenCV Mat containing the image to process
     * @return Vector of Hand objects (0-2 hands typically)
     * @authors Hasit
     */
    std::vector<Hand> detectHands(const cv::Mat& frame);
    /**
         * @brief Check if HandTracker was initialized successfully
         * @return true if Python subprocess is running
         * @authors Hasit
         */

    bool isInitialized() const { return initialized_; }

private:
    /**
         * @brief Launches the Python MediaPipe detection subprocess.
         * This function starts the Python script responsible for performing
         * hand detection and establishes a bidirectional communication pipe
         * between the C++ program and the Python process.
         * @return true if successful, false otherwise
         * @authors Hasit
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


    bool initialized_;  // Flag indicating successful initialization

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