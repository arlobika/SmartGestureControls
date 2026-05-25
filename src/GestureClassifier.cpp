/**
 * @file GestureClassifier.cpp
 * @brief Implements gesture classification based on MediaPipe hand landmarks.
 *
 * Landmark reference:
 *   0: Wrist
 *   1-4: Thumb      (CMC, MCP, IP, TIP)
 *   5-8: Index      (MCP, PIP, DIP, TIP)
 *   9-12: Middle    (MCP, PIP, DIP, TIP)
 *   13-16: Ring     (MCP, PIP, DIP, TIP)
 *   17-20: Pinky    (MCP, PIP, DIP, TIP)
 *
 * Finger extension: fingertip.y < PIP_joint.y  (screen Y is inverted)
 * Thumb extension:  |thumb_tip.x - wrist.x| > |thumb_ip.x - wrist.x|
 * Gesture classification is performed by determining whether each finger
 * is extended or folded using relative landmark positions.
 * Rules used:
 * - Finger extension: fingertip.y < PIP_joint.y
 * - Thumb extension: horizontal distance from wrist increases toward thumb tip
 *
 * These rules allow recognition of gestures such as open hand, fist,
 * peace sign, thumbs up, and thumbs down.
 * @authors Hasit, Dhivya, Arlo
 */

#include "GestureClassifier.hpp"
#include <cmath>

/**
* @brief Converts a GestureType value into a readable string.
* This function maps each gesture enumeration value to a human-readable
* name
* @param gesture The gesture type to convert.
* @return A string representing the gesture name.
* @authors Dhivya
*/
std::string gestureToString(GestureType gesture) {
  switch (gesture) {
  case GestureType::OPEN_HAND:
    return "OPEN_HAND";
  case GestureType::FIST:
    return "FIST";
  case GestureType::THUMBS_UP:
    return "THUMBS_UP";
  case GestureType::THUMBS_DOWN:
    return "THUMBS_DOWN";
  case GestureType::PEACE:
    return "PEACE";
  default:
    return "NONE";
  }
}

/**
 * @brief Determines whether a finger is extended.
 * A finger is considered extended when its fingertip is positioned above
 * its PIP joint in the image coordinate system. Since screen coordinates
 * increase downward, a smaller Y value means the point is higher on the
 * screen.
 *
 * This rule applies to the index, middle, ring, and pinky fingers.
 *
 * @param hand The detected hand containing landmark data.
 * @param tipIdx Index of the fingertip landmark.
 * @param pipIdx Index of the PIP joint landmark.
 * @return true if the finger is extended, false otherwise.
 *
 * @author Hasit
 */
bool GestureClassifier::isFingerExtended(const Hand &hand, int tipIdx,
                                         int pipIdx) const {
  // Finger is extended if the tip is above (lower Y) the PIP joint
  return hand.landmarks[tipIdx].y < hand.landmarks[pipIdx].y;
}

/**
 * @brief Determines whether the thumb is extended.
 * The thumb extends sideways rather than vertically like the other
 * fingers. This method compares the horizontal distance between the
 * wrist and the thumb tip against the distance between the wrist and
 * the thumb IP joint.
 * If the thumb tip is farther from the wrist than the IP joint, the
 * thumb is considered extended.
 *
 * @param hand The detected hand containing landmark data.
 * @return true if the thumb is extended, false otherwise.
 * @author Arlo
 */
bool GestureClassifier::isThumbExtended(const Hand &hand) const {
  // Thumb extends sideways, so compare X distances from wrist
  // Tip (4) should be farther from wrist (0) than IP joint (3)
  int wristX = hand.landmarks[0].x;
  int tipDist = std::abs(hand.landmarks[4].x - wristX);
  int ipDist = std::abs(hand.landmarks[3].x - wristX);
  return tipDist > ipDist;
}
/**
 * @brief Classifies the gesture represented by the given hand landmarks.
 * This function analyzes the extension state of each finger and applies
 * a set of rule-based conditions to determine which predefined gesture
 * is being performed.
 *
 * Gesture detection logic:
 * - OPEN_HAND: four or more fingers extended
 * - FIST: no fingers extended and thumb not extended
 * - PEACE: index and middle fingers extended only
 * - THUMBS_UP / THUMBS_DOWN: thumb extended with all other fingers folded
 *
 * If the landmarks do not match any recognized gesture pattern, the
 * function returns GestureType::NONE.
 *
 * @param hand A Hand object containing the 21 MediaPipe landmark points.
 * @return The classified GestureType corresponding to the detected gesture.
 *
 * @author Arlo
 */
GestureType GestureClassifier::classify(const Hand &hand) const {
  // Need all 21 landmarks
  if (hand.landmarks.size() < 21) {
    return GestureType::NONE;
  }

  // Check each finger's extension state
  bool thumb = isThumbExtended(hand);
  bool index = isFingerExtended(hand, 8, 6);   // Index: tip=8, PIP=6
  bool middle = isFingerExtended(hand, 12, 10); // Middle: tip=12, PIP=10
  bool ring = isFingerExtended(hand, 16, 14);   // Ring: tip=16, PIP=14
  bool pinky = isFingerExtended(hand, 20, 18);  // Pinky: tip=20, PIP=18

  int extendedCount = index + middle + ring + pinky; // Thumb handled separately

    /* Gesture Controls */
  // OPEN_HAND: 4 fingers extended (thumb optional)
  if (extendedCount >= 4) {
    return GestureType::OPEN_HAND;
  }

  // FIST: No fingers extended and thumb not extended
  if (extendedCount == 0 && !thumb) {
    return GestureType::FIST;
  }

  // PEACE: Only index + middle extended
  if (index && middle && !ring && !pinky) {
    return GestureType::PEACE;
  }

  // THUMBS_UP / THUMBS_DOWN: Only thumb extended, no other fingers
  if (thumb && extendedCount == 0) {
    // Determine direction: thumb tip above or below wrist
    if (hand.landmarks[4].y < hand.landmarks[2].y) {
      return GestureType::THUMBS_UP;
    } else {
      return GestureType::THUMBS_DOWN;
    }
  }

  return GestureType::NONE;
}
