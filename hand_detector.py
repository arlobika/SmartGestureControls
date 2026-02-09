#!/usr/bin/env python3
"""
Hand Detection Server using MediaPipe

This script runs as a subprocess, communicating with the C++ application via stdin/stdout.
It receives base64-encoded images, runs MediaPipe hand landmark detection,
and returns JSON with hand data.

Communication Protocol:
1. Read image size from stdin (text line)
2. Read base64-encoded JPEG data from stdin
3. Decode base64 → JPEG bytes → numpy array
4. Run MediaPipe hand detection
5. Convert normalized coordinates (0-1) to pixel coordinates
6. Return JSON via stdout

Author: Smart Gesture Controls Team
Date: February 2026
"""

try:
    import mediapipe as mp
    from mediapipe.tasks import python
    from mediapipe.tasks.python import vision
    import cv2
    import json
    import sys
    import numpy as np
    import base64
except ImportError as e:
    print(f"Import error: {e}", file=sys.stderr)
    sys.exit(1)

class HandDetector:
    """
    MediaPipe Hand Landmarker wrapper
    
    Detects hands in images and returns 21 landmarks per hand.
    
    Landmark Indices:
        0: Wrist
        1-4: Thumb (base to tip)
        5-8: Index finger
        9-12: Middle finger
        13-16: Ring finger
        17-20: Pinky
    """
    
    def __init__(self):
        """
        Initialize MediaPipe Hand Landmarker
        
        Loads the hand_landmarker.task model file and creates a detector instance.
        Raises exception if model file not found or initialization fails.
        """
        try:
            # Use MediaPipe Tasks API (newer version)
            # Model path is relative to script location
            base_options = python.BaseOptions(model_asset_path='models/hand_landmarker.task')
            
            # Configure hand detection parameters
            options = vision.HandLandmarkerOptions(
                base_options=base_options,
                num_hands=2,                              # Max hands to detect simultaneously
                min_hand_detection_confidence=0.5,        # Minimum confidence for detection
                min_hand_presence_confidence=0.5,         # Minimum confidence for presence
                min_tracking_confidence=0.5               # Minimum confidence for tracking
            )
            
            # Create detector instance
            self.detector = vision.HandLandmarker.create_from_options(options)
        except Exception as e:
            print(f"Error initializing hands: {e}", file=sys.stderr)
            raise
    
    def detect(self, image_data):
        """
        Detect hands in an image
        
        Args:
            image_data (bytes): JPEG-encoded image data
            
        Returns:
            dict: JSON-serializable dictionary with format:
                {
                    "hands": [
                        {
                            "handedness": "Left" or "Right",
                            "confidence": 0.0 - 1.0,
                            "landmarks": [
                                {"x": int, "y": int, "z": float},
                                ... (21 total landmarks)
                            ]
                        },
                        ... (0-2 hands)
                    ]
                }
                
                Or on error:
                {
                    "error": "error message",
                    "hands": []
                }
        """
        try:
            # Decode JPEG bytes to numpy array (OpenCV format)
            nparr = np.frombuffer(image_data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            
            if frame is None:
                return {"hands": []}
            
            h, w, c = frame.shape
            
            # Convert BGR (OpenCV) to RGB (MediaPipe requirement)
            rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            
            # Create MediaPipe Image object
            mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb_frame)
            
            # Run hand detection
            # Returns detection_result with:
            #   - hand_landmarks: list of landmark lists (normalized 0-1 coordinates)
            #   - handedness: list of classification results (Left/Right)
            detection_result = self.detector.detect(mp_image)
            
            # Convert MediaPipe result to JSON format
            hands_data = []
            if detection_result.hand_landmarks and detection_result.handedness:
                # Iterate through detected hands
                for landmarks, handedness in zip(detection_result.hand_landmarks, detection_result.handedness):
                    hand_info = {
                        "handedness": handedness[0].category_name,  # "Left" or "Right"
                        "confidence": float(handedness[0].score),   # Detection confidence
                        "landmarks": [
                            {
                                "x": int(lm.x * w),  # Convert normalized (0-1) to pixel coordinates
                                "y": int(lm.y * h),
                                "z": float(lm.z)     # Depth (relative to wrist)
                            }
                            for lm in landmarks  # 21 landmarks per hand
                        ]
                    }
                    hands_data.append(hand_info)
            
            return {"hands": hands_data}
        except Exception as e:
            # Return error in JSON format
            return {"error": str(e), "hands": []}

# ===== MAIN LOOP =====
if __name__ == "__main__":
    try:
        # Initialize MediaPipe hand detector
        detector = HandDetector()
        
        # Signal to C++ that we're ready to receive frames
        print("READY", flush=True)
        
        frame_count = 0
        
        # Main processing loop
        # Reads frames from stdin, processes them, outputs JSON to stdout
        while True:
            try:
                # ===== READ REQUEST FROM C++ =====
                # Format: [SIZE]\n[BASE64_DATA]\n
                
                # Read image size (number of base64 characters)
                size_line = sys.stdin.readline()
                if not size_line:
                    break  # stdin closed, exit
                
                image_size = int(size_line.strip())
                
                # Read base64 encoded image data
                image_b64_data = sys.stdin.read(image_size)
                
                # Read the trailing newline
                sys.stdin.read(1)
                
                # ===== DECODE IMAGE =====
                # Decode base64 → JPEG bytes
                image_data = base64.b64decode(image_b64_data)
                
                # ===== DETECT HANDS =====
                result = detector.detect(image_data)
                
                # ===== SEND RESPONSE TO C++ =====
                # Format: JSON\n
                json_str = json.dumps(result)
                print(json_str, flush=True)  # flush ensures immediate delivery
                
                # Debug logging every 30 frames
                frame_count += 1
                if frame_count % 30 == 0:
                    print(f"DEBUG: Processed {frame_count} frames", file=sys.stderr, flush=True)
                    
            except KeyboardInterrupt:
                # Graceful shutdown on Ctrl+C
                break
            except Exception as e:
                # Send error to C++ as JSON
                print(json.dumps({"error": str(e), "hands": []}), flush=True)
                
    except Exception as e:
        # Initialization failed
        print(f"FAILED: {e}", flush=True)
        sys.exit(1)



