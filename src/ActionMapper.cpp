/**
 * @file ActionMapper.cpp
 * @brief Implements the ActionMapper class for converting
 * gestures into simulated keyboard input.
 *
 * Maps gestures to simulated keypresses for YouTube control.
 * Cross-platform: uses CGEvent on macOS, SendInput on Windows.
 *
 * Gesture → Key Mapping:
 *   OPEN_HAND  → Space      → Play/Pause
 *   FIST       → M          → Mute/Unmute
 *   THUMBS_UP  → Up Arrow   → Volume Up
 *   THUMBS_DOWN→ Down Arrow → Volume Down
 *   PEACE      → L          → Skip Forward 10s
 * @authors Arlo, Sneh, Dhivya
 */

#include "ActionMapper.hpp"
#include <iostream>

// Platform-specific includes for keyboard simulation
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#elif _WIN32
#include <Windows.h>
#endif
/*
* @brief Constructs an ActionMapper object with a specified cooldown period.
* This constructor initializes the cooldown duration between gesture-triggered
* actions, sets the last recognized gesture to `GestureType::NONE`, and sets
* the last action time far enough in the past to allow the first gesture to
* trigger an action immediately.
* @param cooldownSeconds The minimum number of seconds that must pass before
* another gesture can trigger a new action.
* @authors Arlo
*/
ActionMapper::ActionMapper(double cooldownSeconds)
    : cooldownSeconds_(cooldownSeconds), lastGesture_(GestureType::NONE),
      lastActionTime_(std::chrono::steady_clock::now() -
                      std::chrono::seconds(10)) // Allow immediate first action
{}
/*
 * @brief Simulates a keyboard press for the specified key code.
 * This function sends both a key press and key release event to the operating
 * system so that the target application receives the input as a normal keyboard
 * action.
 * Platform behavior:
 * - On macOS, the function uses `CGEventCreateKeyboardEvent` and `CGEventPost`.
 * - On Windows, the function uses the `SendInput` API with separate key-down
 *   and key-up events.
 * @param keyCode (The platform specific key code to simulate
 * @authors Dhivya
*/
void ActionMapper::simulateKeypress(int keyCode) {
#ifdef __APPLE__
  // macOS: Use Core Graphics events
  CGEventRef keyDown =
      CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, true);
  CGEventRef keyUp =
      CGEventCreateKeyboardEvent(NULL, (CGKeyCode)keyCode, false);

  if (keyDown && keyUp) {
    CGEventPost(kCGHIDEventTap, keyDown);
    CGEventPost(kCGHIDEventTap, keyUp);
  }

  if (keyDown)
    CFRelease(keyDown);
  if (keyUp)
    CFRelease(keyUp);

#elif _WIN32
  // Windows: Use SendInput API
  INPUT inputs[2] = {};

  // Key down
  inputs[0].type = INPUT_KEYBOARD;
  inputs[0].ki.wVk = static_cast<WORD>(keyCode);

  // Key up
  inputs[1].type = INPUT_KEYBOARD;
  inputs[1].ki.wVk = static_cast<WORD>(keyCode);
  inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

  SendInput(2, inputs, sizeof(INPUT));
#endif
}
/**
 * @brief Returns the remaining cooldown time before another action may occur.
 * This function computes the time elapsed since the last triggered action and
 * subtracts it from the configured cooldown duration. If the cooldown has
 * already expired, the function returns `0.0`.
 *
 * @return The number of seconds remaining in the cooldown period. Returns
 * `0.0` if the cooldown has expired.
 *
 * @author Sneh
*/
double ActionMapper::getCooldownRemaining() const {
  auto now = std::chrono::steady_clock::now();
  double elapsed =
      std::chrono::duration<double>(now - lastActionTime_).count();
  double remaining = cooldownSeconds_ - elapsed;
  return remaining > 0 ? remaining : 0.0;
}

/**
 * @brief Processes a recognized gesture and triggers the corresponding action.
 * This function checks whether the detected gesture is valid and whether the
 * cooldown period has expired. If the gesture is recognized and the cooldown
 * allows it, the function maps the gesture to a platform-specific keyboard
 * event and simulates the associated keypress.
 * Gesture mapping:
 * - `OPEN_HAND` triggers Space for play/pause
 * - `FIST` triggers M for mute/unmute
 * - `THUMBS_UP` triggers Up Arrow for volume increase
 * - `THUMBS_DOWN` triggers Down Arrow for volume decrease
 * - `PEACE` triggers L for skipping forward
 *
 * After successfully triggering an action, the function updates the cooldown
 * state and stores the most recently processed gesture.
 *
 * @param gesture The recognized gesture to process.
 * @return A string describing the action performed. Returns an empty string
 * if no action was taken because the gesture was `NONE`, unrecognized, or
 * still within the cooldown period.
 *
 * @author Dhivya
 */

  std::string ActionMapper::handleGesture(GestureType gesture) {
  // Ignore NONE gestures
  if (gesture == GestureType::NONE) {
    return "";
  }

  // Check cooldown
  auto now = std::chrono::steady_clock::now();
  double elapsed =
      std::chrono::duration<double>(now - lastActionTime_).count();

  if (elapsed < cooldownSeconds_) {
    return ""; // Still on cooldown
  }

  // Map gesture to keypress
  // macOS uses CGKeyCode values, Windows uses VK_ virtual key codes
  std::string action;
  switch (gesture) {
  case GestureType::OPEN_HAND:
#ifdef __APPLE__
    simulateKeypress(49);  // macOS: Space
#elif _WIN32
    simulateKeypress(VK_SPACE);  // Windows: Space
#endif
    action = "Play/Pause (Space)";
    break;
  case GestureType::FIST:
#ifdef __APPLE__
    simulateKeypress(46);  // macOS: M
#elif _WIN32
    simulateKeypress(0x4D);  // Windows: M
#endif
    action = "Mute (M)";
    break;
  case GestureType::THUMBS_UP:
#ifdef __APPLE__
    simulateKeypress(126);  // macOS: Up Arrow
#elif _WIN32
    simulateKeypress(VK_UP);  // Windows: Up Arrow
#endif
    action = "Volume Up";
    break;
  case GestureType::THUMBS_DOWN:
#ifdef __APPLE__
    simulateKeypress(125);  // macOS: Down Arrow
#elif _WIN32
    simulateKeypress(VK_DOWN);  // Windows: Down Arrow
#endif
    action = "Volume Down";
    break;
  case GestureType::PEACE:
#ifdef __APPLE__
    simulateKeypress(37);  // macOS: L
#elif _WIN32
    simulateKeypress(0x4C);  // Windows: L
#endif
    action = "Skip Forward (L)";
    break;
  default:
    return "";
  }

  // Update cooldown state
  lastActionTime_ = now;
  lastGesture_ = gesture;

  std::cout << "[ACTION] " << action << std::endl;
  return action;
}
