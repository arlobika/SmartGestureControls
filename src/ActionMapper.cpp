/**
 * ActionMapper.cpp
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
 */

#include "ActionMapper.hpp"
#include <iostream>

// Platform-specific includes for keyboard simulation
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#elif _WIN32
#include <Windows.h>
#endif

ActionMapper::ActionMapper(double cooldownSeconds)
    : cooldownSeconds_(cooldownSeconds), lastGesture_(GestureType::NONE),
      lastActionTime_(std::chrono::steady_clock::now() -
                      std::chrono::seconds(10)) // Allow immediate first action
{}

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

double ActionMapper::getCooldownRemaining() const {
  auto now = std::chrono::steady_clock::now();
  double elapsed =
      std::chrono::duration<double>(now - lastActionTime_).count();
  double remaining = cooldownSeconds_ - elapsed;
  return remaining > 0 ? remaining : 0.0;
}

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
