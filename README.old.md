# Smart Gesture Controls

## Overview
Smart Gesture Controls is a camera-based system that allows users to control media playback using real-time hand gestures.
The system uses a standard webcam and computer vision techniques to detect hand gestures and map them to media control actions such as play/pause, volume adjustment, and skipping tracks.

The core focus of the project is the software pipeline, implemented in C++, using OpenCV for real-time video capture and MediaPipe for hand landmark detection and gesture recognition.

---

## Features
- Hotkey to enable or disable gesture control
- Real-time webcam video capture
- Hand gesture recognition and tracking
- Gesture-to-action mapping (media controls, navigation)
- Cross-platform support (Windows and macOS)

### Example Gestures
- High five → Play / Pause
- Swipe left / right → Skip backward / forward
- Swipe up / down → Volume control
- Point & pinch → Zoom in / out
- Left-hand and right-hand recognition

---

## Project Structure
.
├── src/            # Source files
├── include/        # Header files
├── assets/         # Assets and demo resources
├── build/          # Build output (ignored by git)
├── CMakeLists.txt
├── README.md
└── .gitignore

---

## Requirements
- C++17 or newer
- CMake (>= 3.16)
- OpenCV
- Webcam
- MediaPipe (integration in progress)

---

## Build Instructions

### macOS
brew install cmake ninja opencv

### Windows
- Install Visual Studio Build Tools (C++)
- Install CMake
- Install OpenCV and configure environment variables

---

## Build & Run
cmake -S . -B build -G Ninja
cmake --build build
./build/app

Press ESC to exit the application.

---

## Development Workflow
- All development is done on feature branches
- Merge Requests are used to merge into main
- Version control is handled using Git and GitLab

---

## Current Status
- [x] Repository setup
- [x] OpenCV webcam capture
- [ ] MediaPipe hand tracking
- [ ] Gesture classification
- [ ] OS-level media control
- [ ] UI feedback and optimization

---

## Notes
This project is developed as part of a group software engineering assignment.
Focus is placed on modularity, real-time performance, and robustness against lighting and positioning variations.
