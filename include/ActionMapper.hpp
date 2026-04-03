/**
 * @file ActionMapper.hpp
 * @brief Defines the ActionMapper class used to convert gestures into keyboard events
 * Maps recognized gestures to macOS keyboard events.
 * Uses CGEventCreateKeyboardEvent to simulate keypresses that control YouTube.
 *
 * Requires macOS Accessibility permissions:
 *   System Settings → Privacy & Security → Accessibility
 * @authors Selahattin, Sneh
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
 * @authors Selahattin, Sneh
 */
class ActionMapper {
public:
  /**
   * @brief Constructs an ActionMapper with a specified cooldown period
   * @param cooldownSeconds Minimum seconds between consecutive actions
   * @author Selahattin
   */
  ActionMapper(double cooldownSeconds = 2.0);

  /**
   * @brief Process a gesture and trigger the mapped action if cooldown has elapsed
   * This function receives a classified gesture and determines whether a
   * keyboard event should be triggered. If the cooldown period has not
   * yet elapsed since the last action, the gesture is ignored
   * @param gesture The classified gesture
   * @return Description of the action taken, or empty string if on cooldown
   * @authors Sneh
   */
  std::string handleGesture(GestureType gesture);

  /**
   * @brief Returns the remaining cooldown time before another action can occur.
   * @return Get the current cooldown remaining in seconds
  * @authors Selahattin
   */
  double getCooldownRemaining() const;

private:
  /**
   * @brief Simulate a keypress on macOS using CGEvents
   * @param keyCode The macOS virtual key code to simulate
   * @authors Sneh
   */

#ifdef __APPLE__
  /**
   * @brief Simulate a macOS media key press (play/pause, next, prev, etc.)
   * Uses NX_SYSDEFINED events — required for media keys, which cannot
   * be sent via regular CGEventCreateKeyboardEvent calls.
   * @param keyCode One of the NX_KEYTYPE_* constants (e.g. NX_KEYTYPE_PLAY)
  * @authors Selahattin
   */
  void simulateMediaKey(uint32_t keyCode);
#endif
  void simulateKeypress(int keyCode);
  /**
  * @brief minimum cooldown time between gesture-ttriggered actions
  * @authors Selahattin
  */

  double cooldownSeconds_;
  /**
  * @brief the last gesture that triggered an action
  * @authors Selahattin
  */
  GestureType lastGesture_;
  /**
  * @brief timestamp of the last executed action
  * Stored using std::chrono::steady_clock to ensure stable timing
  * @authors Sneh
  */
  std::chrono::steady_clock::time_point lastActionTime_;
};
