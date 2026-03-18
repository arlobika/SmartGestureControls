/**
 * ActionMapper.hpp
 *
 * Maps recognized gestures to macOS keyboard events.
 * Uses CGEventCreateKeyboardEvent to simulate keypresses that control YouTube.
 *
 * Requires macOS Accessibility permissions:
 *   System Settings → Privacy & Security → Accessibility
 */

#pragma once

#include "GestureClassifier.hpp"
#include <chrono>
#include <string>

/**
 * ActionMapper
 *
 * Handles the mapping from GestureType → simulated keypress.
 * Includes a cooldown to prevent repeated triggering from a held gesture.
 */
class ActionMapper {
public:
  /**
   * Constructor
   * @param cooldownSeconds Minimum seconds between consecutive actions
   */
  ActionMapper(double cooldownSeconds = 2.0);

  /**
   * Process a gesture and trigger the mapped action if cooldown has elapsed
   *
   * @param gesture The classified gesture
   * @return Description of the action taken, or empty string if on cooldown
   */
  std::string handleGesture(GestureType gesture);

  /**
   * Get the current cooldown remaining in seconds
   */
  double getCooldownRemaining() const;

private:
  /**
   * Simulate a keypress on macOS using CGEvents
   * @param keyCode The macOS virtual key code
   */
  void simulateKeypress(int keyCode);

  double cooldownSeconds_;
  GestureType lastGesture_;
  std::chrono::steady_clock::time_point lastActionTime_;
};
