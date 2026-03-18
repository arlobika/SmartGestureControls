# Smart Gesture Controls - Hand Tracking

Real-time hand tracking using **MediaPipe** and **OpenCV** with a hybrid C++/Python architecture.

## Quick Start

```bash
# Build and run
# Navigate to the project root directory
mkdir -p build && cd build
cmake .. && make
cd ..
./build/app
```

**Controls:** Press ESC to exit

## Features

- ✅ Real-time hand detection (up to 2 hands)
- ✅ 21 landmark points per hand
- ✅ Skeleton visualization with finger connections
- ✅ Left/Right hand classification
- ✅ Confidence scores
- ✅ ~33 FPS performance

## Architecture

```
C++ (OpenCV) ←→ Python Subprocess (MediaPipe)
    main.cpp        hand_detector.py
       ↓                    ↓
  HandTracker  →→→  Hand Detection
       ↓                    ↓
  JSON Parse  ←←←  JSON Response
       ↓
  Rendering
```

## Project Structure

```
group31/
├── CMakeLists.txt              # Build config
├── README.md                   # This file
├── DOCUMENTATION.md            # Full documentation
├── hand_detector.py            # Python ML inference
├── include/
│   ├── HandTracker.hpp         # Hand detection interface
│   ├── GestureClassifier.hpp   # Gesture recognition (future)
│   └── ActionMapper.hpp        # Action mapping (future)
├── src/
│   ├── main.cpp               # Main application loop
│   ├── HandTracker.cpp        # HandTracker implementation
│   ├── GestureClassifier.cpp  # Gesture logic (future)
│   └── ActionMapper.cpp       # Action mapping (future)
├── models/
│   └── hand_landmarker.task   # MediaPipe model
└── build/
    └── app                    # Executable
```

## Hand Landmarks

MediaPipe detects 21 landmarks per hand:

```
        8   12  16  20  (fingertips)
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
```

## Customization

### Change Number of Hands

Edit [hand_detector.py](hand_detector.py):
```python
options = vision.HandLandmarkerOptions(
    base_options=base_options,
    num_hands=4  # Change from 2 to 4
)
```

### Adjust Colors

Edit [src/main.cpp](src/main.cpp):
```cpp
cv::Scalar landmark_color(0, 255, 0);  // Green (BGR)
cv::Scalar skeleton_color(255, 0, 0);  // Blue
```

### Change Frame Rate

Edit [src/main.cpp](src/main.cpp):
```cpp
cv::waitKey(30)  // Change 30ms to your preferred delay
```

## Performance Tips

1. **Reduce resolution:**
   ```cpp
   cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
   cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
   ```

2. **Skip frames:**
   ```cpp
   if (frame_count++ % 2 == 0) {
       hands = tracker.detectHands(frame);
   }
   ```

3. **Increase confidence threshold:**
   ```python
   min_detection_confidence=0.7  # Higher = fewer false positives
   ```

## Documentation

See [DOCUMENTATION.md](DOCUMENTATION.md) for:
- Detailed architecture explanation
- Communication protocol details
- JSON parsing implementation
- Gesture recognition guide
- Troubleshooting tips

## Dependencies

- **C++17** or later
- **CMake** 3.16+
- **OpenCV** 4.x
- **Python** 3.9+
- **MediaPipe** 0.10+

## Building from Scratch

### macOS
```bash
# Install dependencies
brew install cmake opencv

# Set up Python environment
python3 -m venv .venv
source .venv/bin/activate
pip install mediapipe opencv-python

# Build C++ application
mkdir -p build && cd build
cmake ..
make

# Run
cd ..
./build/app
```

### Windows
```bash
# Install dependencies (using vcpkg)
vcpkg install opencv

# Set up Python environment
python -m venv .venv
.venv\Scripts\activate
pip install mediapipe opencv-python

# Build C++ application
mkdir build && cd build
cmake ..
cmake --build .

# Run
cd ..
build\Debug\app.exe
```

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "Could not open webcam" | Check camera permissions in System Preferences |
| "Failed to initialize HandTracker" | Verify model file exists: `models/hand_landmarker.task` |
| No landmarks showing | Check debug output in terminal |
| Application lags | Reduce resolution or skip frames (see Performance Tips) |
| Python process error | Activate venv: `source .venv/bin/activate` |

## Future Enhancements

- [ ] Gesture recognition (fist, peace sign, thumbs up)
- [ ] Action mapping (volume control, mouse control)
- [ ] Multi-threading for better performance
- [ ] Recording and playback
- [ ] Custom gestures training

## License

Educational purposes. MediaPipe is Apache 2.0 licensed.

## Original Project Goals

This project was initially conceived as a media control system. See [README.old.md](README.old.md) for the original project vision including:
- Hotkey-enabled gesture control
- Media playback controls (play/pause, volume, skip)
- Cross-platform support

## Author

Smart Gesture Controls Team - Group 31

---

**Need help?** Check [DOCUMENTATION.md](DOCUMENTATION.md) for comprehensive guides and implementation details.


Running:
    - mkdir -p build && cd build && cmake .. && make && cd ..
    - ./build/app
