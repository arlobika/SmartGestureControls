/**
 * Smart Gesture Controls - Main Application
 * 
 * This application captures video from a webcam, detects hands using MediaPipe
 * (via Python subprocess), and visualizes the detected hand landmarks in real-time.
 * 
 * Architecture:
 * 1. OpenCV captures frames from webcam
 * 2. HandTracker sends frames to Python subprocess for ML inference
 * 3. Python returns JSON with hand landmarks
 * 4. Main loop draws landmarks and skeleton on frame
 * 5. OpenCV displays the result
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include "HandTracker.hpp"

int main() {
    // Initialize webcam capture
    // CAP_AVFOUNDATION is required for macOS camera access
    // 0 = default camera (usually built-in webcam)
    cv::VideoCapture cap(0, cv::CAP_AVFOUNDATION);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam\n";
        return 1;
    }

    // Create HandTracker instance
    // This starts a Python subprocess that runs MediaPipe hand detection
    HandTracker tracker;
    if (!tracker.isInitialized()) {
        std::cerr << "Error: Failed to initialize HandTracker\n";
        return 1;
    }

    cv::Mat frame;  // Frame buffer for captured images
    int hand_count_prev = 0;  // Unused, kept for future smoothing logic
    
    // FPS calculation variables
    auto last_time = std::chrono::high_resolution_clock::now();
    double fps = 0.0;
    int frame_counter = 0;
    
    // ===== MAIN LOOP =====
    while (true) {
        // Calculate FPS
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = current_time - last_time;
        frame_counter++;
        
        // Update FPS every 10 frames for smoother display
        if (frame_counter >= 10) {
            fps = frame_counter / elapsed.count();
            last_time = current_time;
            frame_counter = 0;
        }
        // Capture frame from webcam
        cap >> frame;
        if (frame.empty()) break;  // Exit if camera disconnected

        // Detect hands in current frame
        // Returns vector of Hand objects, each with 21 landmarks
        auto hands = tracker.detectHands(frame);
        
        // ===== VISUALIZATION =====
        // Draw hand landmarks and skeleton for each detected hand
        for (size_t h = 0; h < hands.size(); ++h) {
            const auto& hand = hands[h];
            
            // ===== DRAW SKELETON (CONNECTIONS) =====
            // Draw connections FIRST so they appear behind the landmark dots
            // 
            // MediaPipe Hand Landmark Indices:
            //   0: Wrist
            //   1-4: Thumb (from base to tip)
            //   5-8: Index finger
            //   9-12: Middle finger
            //   13-16: Ring finger
            //   17-20: Pinky
            const std::vector<std::pair<int, int>> connections = {
                {0, 1}, {1, 2}, {2, 3}, {3, 4},      // Thumb
                {0, 5}, {5, 6}, {6, 7}, {7, 8},      // Index finger
                {0, 9}, {9, 10}, {10, 11}, {11, 12}, // Middle finger
                {0, 13}, {13, 14}, {14, 15}, {15, 16}, // Ring finger
                {0, 17}, {17, 18}, {18, 19}, {19, 20}  // Pinky
            };
            
            // Draw each connection as a line
            for (const auto& [start, end] : connections) {
                if (start < hand.landmarks.size() && end < hand.landmarks.size()) {
                    const auto& p1 = hand.landmarks[start];
                    const auto& p2 = hand.landmarks[end];
                    
                    // Boundary check: only draw if both points are within frame
                    if (p1.x > 0 && p1.y > 0 && p2.x > 0 && p2.y > 0 &&
                        p1.x < frame.cols && p1.y < frame.rows &&
                        p2.x < frame.cols && p2.y < frame.rows) {
                        // Draw blue line (BGR: 255, 0, 0), thickness 2
                        cv::line(frame, p1, p2, cv::Scalar(255, 0, 0), 2);
                    }
                }
            }
            
            // ===== DRAW LANDMARKS (DOTS) =====
            // Draw each of the 21 hand landmarks as a circle
            for (size_t i = 0; i < hand.landmarks.size(); ++i) {
                const auto& lm = hand.landmarks[i];
                
                // Boundary check
                if (lm.x > 0 && lm.y > 0 && lm.x < frame.cols && lm.y < frame.rows) {
                    // Draw green filled circle (radius 5)
                    cv::circle(frame, lm, 5, cv::Scalar(0, 255, 0), -1);
                    // Draw white outline for better visibility
                    cv::circle(frame, lm, 5, cv::Scalar(255, 255, 255), 1);
                }
            }
            
            // ===== DISPLAY HAND INFO =====
            // Show hand type and confidence at bottom of screen
            // Multiple hands are stacked vertically
            int y_offset = frame.rows - 40 - (h * 35);  // Position from bottom
            std::string info = hand.handedness + " Hand (" + 
                             std::to_string(static_cast<int>(hand.confidence * 100)) + "%)";
            cv::putText(frame, info, cv::Point(10, y_offset), 
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        }
        
        // ===== STATUS DISPLAY =====
        // Display detection status in top left corner
        if (hands.empty()) {
            // No hands detected - show red text
            cv::putText(frame, "No hand detected", cv::Point(10, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 255), 2);
        } else {
            // Show number of hands detected
            std::string status = "Hands detected: " + std::to_string(hands.size());
            cv::putText(frame, status, cv::Point(10, 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
        }
        
        // FPS counter in top right
        std::string fps_text = "FPS: " + std::to_string(static_cast<int>(fps));
        cv::putText(frame, fps_text, cv::Point(frame.cols - 120, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
        
        // Exit instruction below FPS
        cv::putText(frame, "Press ESC to exit", cv::Point(frame.cols - 200, 60),
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);

        // Display the frame in a window
        cv::imshow("Smart Gesture Controls - Hand Tracking", frame);
        
        // Wait 30ms and check for ESC key (ASCII 27)
        // 30ms delay = ~33 FPS maximum, reduces CPU usage
        if (cv::waitKey(30) == 27) break;
    }

    // Cleanup: close all OpenCV windows
    cv::destroyAllWindows();
    
    // HandTracker destructor will automatically:
    // - Close the Python subprocess pipe
    // - Terminate the Python process
    
    return 0;
}
