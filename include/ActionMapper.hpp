/**
 * @file ActionMapper.hpp
 * @brief Defines the ActionMapper class used to convert gestures into keyboard events
 * Maps recognized gestures to macOS keyboard events.
 * Uses CGEventCreateKeyboardEvent to simulate keypresses that control YouTube.
 *
 * Requires macOS Accessibility permissions:
 *   System Settings → Privacy & Security → Accessibility
 * @authors
 */

#pragma once

#include "GestureClassifier.hpp"
#include <chrono>
#include <string>

/**
 * @class ActionMapper
 * @brief Maps recognized gestures to simulated macOS keyboard events
 * Handles the mapping from GestureType → simulated keypress.
 * Includes a cooldown to prevent repeated triggering from a held gesture.
 * @authors
 */
class ActionMapper {
public:
  /**
   * @brief Constructs an ActionMapper with a specified cooldown period
   * @param cooldownSeconds Minimum seconds between consecutive actions
   * @author
   */
  ActionMapper(double cooldownSeconds = 2.0);

  /**
   * @brief Process a gesture and trigger the mapped action if cooldown has elapsed
   * This function receives a classified gesture and determines whether a
   * keyboard event should be triggered. If the cooldown period has not
   * yet elapsed since the last action, the gesture is ignored
   * @param gesture The classified gesture
   * @return Description of the action taken, or empty string if on cooldown
   * @authors
   */
  std::string handleGesture(GestureType gesture);

  /**
   * @brief Returns the remaining cooldown time before another action can occur.
   * @return Get the current cooldown remaining in seconds
   * @authors
   */
  double getCooldownRemaining() const;

private:
  /**
   * @brief Simulate a keypress on macOS using CGEvents
   * @param keyCode The macOS virtual key code to simulate
   * @authors
   */
  void simulateKeypress(int keyCode);
  /**
  * @brief minimum cooldown time between gesture-ttriggered actions
  * @authors
  */
  double cooldownSeconds_;
  /**
  * @brief the last gesture that triggered an action
  * @authors
  */
  GestureType lastGesture_;
  /**
  * @brief timestamp of the last executed action
  * Stored using std::chrono::steady_clock to ensure stable timing
  * @authors
  */
  std::chrono::steady_clock::time_point lastActionTime_;
};
