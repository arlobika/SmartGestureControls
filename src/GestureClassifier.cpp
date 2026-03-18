/**
 * GestureClassifier.cpp
 *
 * Implements gesture classification using MediaPipe hand landmarks.
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
 */

#include "GestureClassifier.hpp"
#include <cmath>

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

bool GestureClassifier::isFingerExtended(const Hand &hand, int tipIdx,
                                         int pipIdx) const {
  // Finger is extended if the tip is above (lower Y) the PIP joint
  return hand.landmarks[tipIdx].y < hand.landmarks[pipIdx].y;
}

bool GestureClassifier::isThumbExtended(const Hand &hand) const {
  // Thumb extends sideways, so compare X distances from wrist
  // Tip (4) should be farther from wrist (0) than IP joint (3)
  int wristX = hand.landmarks[0].x;
  int tipDist = std::abs(hand.landmarks[4].x - wristX);
  int ipDist = std::abs(hand.landmarks[3].x - wristX);
  return tipDist > ipDist;
}

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

  // ===== GESTURE RULES =====

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
