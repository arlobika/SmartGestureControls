# Smart Gesture Controls - Hand Tracking Documentation

## Overview

This project implements real-time hand tracking using **MediaPipe** for hand landmark detection and **OpenCV** for video capture and visualization. The system uses a hybrid C++/Python architecture where Python handles the ML inference and C++ handles the video processing and UI.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         C++ Application                      │
│  ┌────────────┐      ┌──────────────┐      ┌─────────────┐ │
│  │   main.cpp │ ───> │ HandTracker  │ ───> │   OpenCV    │ │
│  │            │      │   (C++)      │      │  Rendering  │ │
│  └────────────┘      └──────┬───────┘      └─────────────┘ │
│                              │                               │
└──────────────────────────────┼───────────────────────────────┘
                               │ IPC (pipes)
                               │ Base64 + JSON
                               ▼
┌──────────────────────────────────────────────────────────────┐
│                    Python Subprocess                         │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  hand_detector.py                                      │  │
│  │  • Reads base64-encoded JPEG frames from stdin        │  │
│  │  • Runs MediaPipe hand landmark detection             │  │
│  │  • Returns JSON with hand data via stdout             │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
```

---

## Components

### 1. **main.cpp** - Main Application Loop

**Purpose:** Captures video, detects hands, and renders visualization.

**Key Functions:**
- Initializes webcam capture using OpenCV
- Creates `HandTracker` instance
- Main loop:
  - Captures frame from webcam
  - Calls `tracker.detectHands(frame)` to get hand landmarks
  - Draws landmarks and skeleton on frame
  - Displays UI with hand information
  - Handles keyboard input (ESC to exit)

**Frame Rate Control:** 30ms delay between frames to prevent lag and reduce CPU usage.

---

### 2. **HandTracker (C++)** - Python Process Manager

**Files:** `include/HandTracker.hpp`, `src/HandTracker.cpp`

#### **HandTracker Class**

```cpp
class HandTracker {
public:
    HandTracker();              // Starts Python subprocess
    ~HandTracker();             // Cleans up subprocess
    std::vector<Hand> detectHands(const cv::Mat& frame);
    bool isInitialized() const;
    
private:
    FILE* python_process_;      // Pipe to Python subprocess
    bool initialized_;
};
```

#### **Hand Structure**

```cpp
struct Hand {
    std::vector<cv::Point> landmarks;  // 21 hand landmarks
    float confidence;                  // Detection confidence (0-1)
    std::string handedness;           // "Left" or "Right"
};
```

#### **Communication Protocol**

1. **C++ → Python (Request):**
   ```
   [SIZE]\n
   [BASE64_ENCODED_JPEG_DATA]\n
   ```
   - SIZE: Number of base64 characters
   - DATA: Base64-encoded JPEG image

2. **Python → C++ (Response):**
   ```json
   {"hands": [{"handedness": "Right", "confidence": 0.95, "landmarks": [...]}]}\n
   ```

#### **JSON Parsing**

The `detectHands()` method implements a custom JSON parser:
- Finds the `"hands"` array using bracket matching
- Parses each hand object by tracking brace depth
- Extracts handedness, confidence, and landmarks
- Only adds hands with valid landmark data

---

### 3. **hand_detector.py** - MediaPipe Inference

**Purpose:** Standalone Python process that runs MediaPipe hand landmark detection.

#### **MediaPipe Hand Landmarks**

MediaPipe detects **21 landmarks per hand**:

```
        8   12  16  20      (fingertips)
        |   |   |   |
        7   11  15  19
        |   |   |   |
        6   10  14  18
        |   |   |   |
    4   5   9   13  17
    |   └───┴────┴───┘
    3       (palm)
    |
    2       0 = wrist
    |      /
    1     /
    |    /
    0───┘
  
  Fingers: Thumb(1-4), Index(5-8), Middle(9-12), Ring(13-16), Pinky(17-20)
```

#### **HandDetector Class**

```python
class HandDetector:
    def __init__(self):
        # Loads hand_landmarker.task model
        # Creates MediaPipe HandLandmarker
    
    def detect(self, image_data):
        # Decodes JPEG from bytes
        # Converts BGR → RGB
        # Runs MediaPipe detection
        # Returns JSON with all landmarks
```

#### **Detection Pipeline**

1. Read image size from stdin
2. Read base64-encoded image data
3. Decode base64 → JPEG bytes
4. Decode JPEG → numpy array (OpenCV)
5. Convert BGR → RGB (MediaPipe requirement)
6. Create MediaPipe Image object
7. Run `detector.detect(mp_image)`
8. Convert normalized coordinates (0-1) to pixel coordinates
9. Return JSON with results

---

## Hand Landmark Skeleton Connections

The skeleton is drawn by connecting specific landmark pairs:

```cpp
// Defined in main.cpp
const std::vector<std::pair<int, int>> connections = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4},      // Thumb
    {0, 5}, {5, 6}, {6, 7}, {7, 8},      // Index finger
    {0, 9}, {9, 10}, {10, 11}, {11, 12}, // Middle finger
    {0, 13}, {13, 14}, {14, 15}, {15, 16}, // Ring finger
    {0, 17}, {17, 18}, {18, 19}, {19, 20}  // Pinky
};
```

**Drawing Order:**
1. First: Draw blue lines (skeleton) - background layer
2. Second: Draw green circles (landmarks) - foreground layer
3. Third: Add white outline to circles for visibility

---

## File Structure

```
group31/
├── CMakeLists.txt              # Build configuration
├── hand_detector.py            # Python MediaPipe inference script
├── README.md                   # Project overview
├── DOCUMENTATION.md            # This file
│
├── include/
│   ├── HandTracker.hpp         # HandTracker class declaration
│   ├── GestureClassifier.hpp   # (Empty - for future use)
│   └── ActionMapper.hpp        # (Empty - for future use)
│
├── src/
│   ├── main.cpp               # Main application loop
│   ├── HandTracker.cpp        # HandTracker implementation
│   ├── GestureClassifier.cpp  # (Empty - for future use)
│   └── ActionMapper.cpp       # (Empty - for future use)
│
├── models/
│   └── hand_landmarker.task   # MediaPipe hand detection model
│
├── build/                     # Build artifacts
│   └── app                    # Compiled executable
│
└── .venv/                     # Python virtual environment
    └── bin/python3            # Python interpreter with MediaPipe
```

---

## Building and Running

### Prerequisites

- **C++ Compiler:** Apple Clang 17+ or GCC
- **CMake:** 3.16+
- **OpenCV:** 4.x
- **Python:** 3.9+
- **MediaPipe:** 0.10+

### Build Steps

```bash
# 1. Install OpenCV (if not already installed)
brew install opencv

# 2. Set up Python environment
python3 -m venv .venv
source .venv/bin/activate
pip install mediapipe opencv-python

# 3. Build C++ application
mkdir -p build && cd build
cmake ..
make

# 4. Run
cd ..
./build/app
```

### Controls

- **ESC:** Exit application
- Frame rate: ~33 FPS (30ms delay)

---

## Performance Optimization Tips

### 1. **Reduce Frame Processing**

Skip frames to reduce load:
```cpp
// In main.cpp main loop
static int frame_skip = 0;
if (frame_skip++ % 2 == 0) {  // Process every other frame
    auto hands = tracker.detectHands(frame);
} else {
    // Reuse previous results
}
```

### 2. **Lower Camera Resolution**

```cpp
// In main.cpp after VideoCapture initialization
cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
```

### 3. **Adjust JPEG Quality**

```cpp
// In HandTracker.cpp detectHands()
std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 75}; // Lower = faster
cv::imencode(".jpg", frame, buf, params);
```

### 4. **Increase Detection Confidence**

```python
# In hand_detector.py HandDetector.__init__()
options = vision.HandLandmarkerOptions(
    base_options=base_options,
    min_detection_confidence=0.7,  # Higher = fewer false positives
    min_tracking_confidence=0.7
)
```

---

## Adding Support for More Hands

MediaPipe supports detecting up to **2 hands** by default. To increase:

### Python Side (hand_detector.py)

```python
# In HandDetector.__init__()
base_options = python.BaseOptions(model_asset_path='models/hand_landmarker.task')
options = vision.HandLandmarkerOptions(
    base_options=base_options,
    num_hands=4  # Change from 2 to 4
)
```

### C++ Side (main.cpp)

The C++ code already handles multiple hands via a loop:
```cpp
for (size_t h = 0; h < hands.size(); ++h) {
    const auto& hand = hands[h];
    // Draw landmarks and skeleton
}
```

**Note:** More hands = more CPU usage. Test performance before deploying.

---

## UI Customization

### Color Scheme

```cpp
// In main.cpp, change these cv::Scalar values:
cv::Scalar landmark_color(0, 255, 0);     // Green landmarks (BGR)
cv::Scalar skeleton_color(255, 0, 0);     // Blue skeleton
cv::Scalar outline_color(255, 255, 255);  // White outline
cv::Scalar text_color(0, 255, 0);         // Green text
```

### Font and Text Size

```cpp
// Status text
cv::putText(frame, status, cv::Point(10, 30),
    cv::FONT_HERSHEY_SIMPLEX,  // Font type
    0.8,                        // Scale (increase for larger text)
    cv::Scalar(0, 255, 0),     // Color
    2                           // Thickness
);
```

### Landmark Size

```cpp
// Change circle radius
cv::circle(frame, lm, 5,  // Change 5 to larger value
    cv::Scalar(0, 255, 0), -1);
```

---

## Troubleshooting

### "HandTracker initialized successfully" but no landmarks

**Cause:** Python subprocess started but communication failed.

**Fix:**
1. Check debug output in terminal
2. Verify model path: `models/hand_landmarker.task` exists
3. Ensure Python has MediaPipe: `pip install mediapipe`

### Lag/Stuttering

**Causes:**
- High resolution camera
- Too many hands being tracked
- Slow base64 encoding

**Fixes:**
- Reduce camera resolution
- Decrease `num_hands` in Python
- Increase `cv::waitKey()` delay in main.cpp

### "21 hands detected" with only 1-2 hands

**Cause:** JSON parsing incorrectly counting landmark objects as hands.

**Status:** Fixed in latest version by using depth-first brace matching.

---

## Future Enhancements

### 1. **Gesture Recognition** (GestureClassifier.cpp)

Classify hand poses:
- ✊ Fist
- ✋ Open palm
- ✌️ Peace sign
- 👍 Thumbs up

Implementation:
```cpp
enum Gesture { FIST, OPEN_PALM, PEACE, THUMBS_UP, UNKNOWN };

Gesture classifyGesture(const Hand& hand) {
    // Calculate angles between fingers
    // Use finger tip positions relative to palm
    // Return detected gesture
}
```

### 2. **Action Mapping** (ActionMapper.cpp)

Map gestures to actions:
- Volume control
- Mouse movement
- Keyboard shortcuts

### 3. **Multi-threading**

Separate threads for:
- Frame capture
- Hand detection (Python communication)
- Rendering

### 4. **Save/Load Configurations**

JSON config file for:
- Camera settings
- Detection thresholds
- UI preferences

---

## Technical Details

### Base64 Encoding

**Why?** Pipes use text mode by default. Binary JPEG data causes encoding errors.

**Solution:** Encode JPEG bytes to base64 ASCII before sending over pipe.

```cpp
// C++ side
std::string base64_encode(const unsigned char* buf, unsigned int buflen);
```

**Performance Impact:** ~33% size increase, but ensures reliability.

### JSON Parsing

**Why not use a library?** Keep dependencies minimal. The custom parser is sufficient for the simple JSON structure returned by MediaPipe.

**Complexity:** O(n) where n is JSON string length.

---

## Common Issues and Solutions

| Issue | Solution |
|-------|----------|
| No camera found | Check `cv::CAP_AVFOUNDATION` works on your system |
| Python process hangs | Add timeout in `startPythonProcess()` |
| Memory leak | Ensure `pclose()` is called in destructor |
| Incorrect hand count | Verify JSON parsing depth tracking |

---

## References

- **MediaPipe Hands:** https://developers.google.com/mediapipe/solutions/vision/hand_landmarker
- **OpenCV VideoCapture:** https://docs.opencv.org/4.x/d8/dfe/classcv_1_1VideoCapture.html
- **CMake Tutorial:** https://cmake.org/cmake/help/latest/guide/tutorial/

---

## License

This project is for educational purposes. MediaPipe is licensed under Apache 2.0.
