/**
 * GestureClassifier.hpp
 *
 * Classifies hand landmarks into discrete gesture types.
 * Uses fingertip vs knuckle positions to determine which fingers are extended.
 */

#pragma once

#include "HandTracker.hpp"
#include <string>

/**
 * Supported gesture types and their mapped YouTube actions
 */
enum class GestureType {
  NONE,        // No recognizable gesture
  OPEN_HAND,   // All fingers extended  → Play/Pause (Space)
  FIST,        // All fingers closed     → Mute (M)
  THUMBS_UP,   // Only thumb extended up → Volume Up (↑)
  THUMBS_DOWN, // Only thumb extended dn → Volume Down (↓)
  PEACE        // Index + middle only    → Skip Forward (L)
};

/**
 * Returns a human-readable name for the gesture
 */
std::string gestureToString(GestureType gesture);

/**
 * GestureClassifier
 *
 * Analyzes the 21 MediaPipe hand landmarks to classify the current gesture.
 * Finger extension is determined by comparing fingertip Y to knuckle Y
 * (lower Y = higher on screen). Thumb uses X-distance since it extends sideways.
 */
class GestureClassifier {
public:
  /**
   * Classify the gesture of a detected hand
   *
   * @param hand Hand object with 21 landmarks in pixel coordinates
   * @return The classified GestureType
   */
  GestureType classify(const Hand &hand) const;

private:
  /**
   * Check if a finger is extended (tip above knuckle)
   *
   * @param hand The hand to check
   * @param tipIdx Landmark index of the fingertip (8, 12, 16, or 20)
   * @param pipIdx Landmark index of the PIP joint (6, 10, 14, or 18)
   * @return true if the finger is extended
   */
  bool isFingerExtended(const Hand &hand, int tipIdx, int pipIdx) const;

  /**
   * Check if the thumb is extended (away from palm)
   *
   * @param hand The hand to check
   * @return true if the thumb is extended
   */
  bool isThumbExtended(const Hand &hand) const;
};
