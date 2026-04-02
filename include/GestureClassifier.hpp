/**
 * @file GestureClassifier.hpp
 * @brief Declares the GestureClassifier class and gesture-related utilities
 * Classifies hand landmarks into discrete gesture types.
 * Uses fingertip vs knuckle positions to determine which fingers are extended.
 * @authors:
 */

#pragma once

#include "HandTracker.hpp"
#include <string>

/**
 * @enum GestureType
 * @brief Represents the set of gestures recognized by the classifier.
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
 * @brief Returns a human-readable name for the gesture
 * @authors:
 */
std::string gestureToString(GestureType gesture);

/**
 * @class GestureClassifier
 * @brief Classifies a detected hand into one of several supported gestures
 * Analyzes the 21 MediaPipe hand landmarks to classify the current gesture.
 * Finger extension is determined by comparing fingertip Y to knuckle Y
 * (lower Y = higher on screen). Thumb uses X-distance since it extends sideways.
 * @author
 */
class GestureClassifier {
public:
  /**
   * @brief Classify the gesture of a detected hand
   *
   * @param hand Hand object with 21 landmarks in pixel coordinates
   * @return The classified GestureType that best matches the current hand pose.
   * @author
   */
  GestureType classify(const Hand &hand) const;

private:
  /**
   * @brief Check if a finger is extended (tip above knuckle)
   * This rule is used for the index, middle, ring, and pinky fingers
   * @param hand The hand to check
   * @param tipIdx Landmark index of the fingertip (8, 12, 16, or 20)
   * @param pipIdx Landmark index of the PIP joint (6, 10, 14, or 18)
   * @return true if the finger is extended
   * @authors
   */
  bool isFingerExtended(const Hand &hand, int tipIdx, int pipIdx) const;

  /**
   * @brief Check if the thumb is extended (away from palm)
   * The method uses the horizontal relationship between thumb landmarks and
   * the rest of the hand to decide whether the thumb is extended.
   * This logic is used when distinguishing gestures such as thumbs up,
   * thumbs down, and a closed fist.
   * @param hand The hand to check
   * @return true if the thumb is extended
   * @authors
   */
  bool isThumbExtended(const Hand &hand) const;
};
