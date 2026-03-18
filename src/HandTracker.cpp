#include "HandTracker.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

// Platform-specific includes
#ifdef _WIN32
#include <Windows.h>
#define popen _popen
#define pclose _pclose
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms)*1000)
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h> // _NSGetExecutablePath
#endif

/**
 * Get the project root directory at runtime.
 * The executable lives in build/, so project root is one level up.
 * Works on macOS, Linux, and Windows without any hardcoded paths.
 */
static std::string getProjectRoot() {
  std::string exePath;

#ifdef _WIN32
  char buf[MAX_PATH];
  GetModuleFileNameA(NULL, buf, MAX_PATH);
  exePath = std::string(buf);
  // Find last backslash to get directory
  size_t lastSlash = exePath.find_last_of("\\");
#elif __APPLE__
  char buf[1024];
  uint32_t size = sizeof(buf);
  _NSGetExecutablePath(buf, &size);
  exePath = std::string(buf);
  size_t lastSlash = exePath.find_last_of("/");
#else
  // Linux: read /proc/self/exe
  char buf[1024];
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
  if (len != -1)
    buf[len] = '\0';
  exePath = std::string(buf);
  size_t lastSlash = exePath.find_last_of("/");
#endif

  // Get the directory containing the executable (e.g., .../group31/build)
  std::string exeDir =
      (lastSlash != std::string::npos) ? exePath.substr(0, lastSlash) : ".";

  // Go one level up from build/ to get project root
#ifdef _WIN32
  return exeDir + "\\..";
#else
  return exeDir + "/..";
#endif
}

// Simple base64 encoding
static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const unsigned char *buf, unsigned int buflen) {
  std::string ret;
  int i = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (buflen--) {
    char_array_3[i++] = *(buf++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] =
          ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] =
          ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i > 0) {
    for (int j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] =
        ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] =
        ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (int j = 0; j <= i; j++)
      ret += base64_chars[char_array_4[j]];

    while (i++ < 3)
      ret += '=';
  }

  return ret;
}

// Simple JSON parser for hand data
struct JsonValue {
  std::string type; // "object", "array", "string", "number", "null"
  std::string value;
};

HandTracker::HandTracker() : initialized_(false), python_process_(nullptr) {
  if (startPythonProcess()) {
    initialized_ = true;
    std::cout << "HandTracker initialized successfully\n";
  } else {
    std::cerr << "Error initializing HandTracker\n";
  }
}

HandTracker::~HandTracker() { stopPythonProcess(); }

bool HandTracker::startPythonProcess() {
  // Resolve paths relative to the project root (one level above build/)
  std::string root = getProjectRoot();

#ifdef _WIN32
  std::string python_path = root + "\\.venv\\Scripts\\python.exe";
  std::string script_path = root + "\\hand_detector.py";
#else
  std::string python_path = root + "/.venv/bin/python3";
  std::string script_path = root + "/hand_detector.py";
#endif

  std::string cmd = python_path + " " + script_path + " 2>&1";

  // Start Python process
  python_process_ = popen(cmd.c_str(), "r+");

  if (!python_process_) {
    std::cerr << "Failed to start Python process\n";
    return false;
  }

  // Set non-blocking mode and wait for "READY" signal
  char buffer[256];
  int attempts = 0;
  while (attempts < 30) { // 30 second timeout
    if (fgets(buffer, sizeof(buffer), python_process_) != nullptr) {
      std::string response(buffer);
      std::cout << "Python response: " << response;
      if (response.find("READY") != std::string::npos) {
        return true;
      } else if (response.find("FAILED") != std::string::npos) {
        return false;
      }
    }
    attempts++;
    sleep_ms(100); // 100ms
  }

  return false;
}

void HandTracker::stopPythonProcess() {
  if (python_process_) {
    pclose(python_process_);
    python_process_ = nullptr;
  }
}

std::vector<Hand> HandTracker::detectHands(const cv::Mat &frame) {
  std::vector<Hand> hands;

  if (!initialized_ || !python_process_)
    return hands;

  static int frame_count = 0;
  frame_count++;

  try {
    // Encode frame to JPEG
    std::vector<uchar> buf;
    cv::imencode(".jpg", frame, buf);

    // Encode to base64
    std::string b64_data = base64_encode(buf.data(), buf.size());

    // Send size and base64 encoded data to Python
    std::string size_cmd = std::to_string(b64_data.size()) + "\n";
    fwrite(size_cmd.c_str(), 1, size_cmd.size(), python_process_);
    fwrite(b64_data.c_str(), 1, b64_data.size(), python_process_);
    fwrite("\n", 1, 1, python_process_);
    fflush(python_process_);

    // Read JSON response
    char buffer[65536];
    if (fgets(buffer, sizeof(buffer), python_process_) != nullptr) {
      std::string json_str(buffer);

      if (frame_count == 1 || frame_count % 60 == 0) {
        // Debug: print first 300 chars of JSON
        std::string preview =
            json_str.substr(0, std::min(size_t(300), json_str.length()));
        std::cerr << "Frame " << frame_count << " JSON preview: " << preview
                  << "\n";
      }

      // Parse JSON: look for the "hands" array
      size_t hands_key = json_str.find("\"hands\"");
      if (hands_key == std::string::npos)
        return hands;

      size_t array_open = json_str.find("[", hands_key);
      if (array_open == std::string::npos)
        return hands;

      // Find the matching ] for the hands array
      int bracket_depth = 0;
      size_t array_close = array_open;
      while (array_close < json_str.length()) {
        if (json_str[array_close] == '[')
          bracket_depth++;
        else if (json_str[array_close] == ']') {
          bracket_depth--;
          if (bracket_depth == 0)
            break;
        }
        array_close++;
      }

      if (bracket_depth != 0)
        return hands;

      std::string hands_content =
          json_str.substr(array_open + 1, array_close - array_open - 1);

      // Parse hand objects at the top level of the hands array
      size_t pos = 0;
      int parsed_hands = 0;
      while (pos < hands_content.length()) {
        // Skip whitespace and commas
        while (pos < hands_content.length() &&
               (hands_content[pos] == ' ' || hands_content[pos] == '\t' ||
                hands_content[pos] == '\n' || hands_content[pos] == ',')) {
          pos++;
        }

        if (pos >= hands_content.length() || hands_content[pos] != '{')
          break;

        // Find matching }
        int brace_depth = 0;
        size_t hand_start = pos;
        size_t hand_end = pos;
        while (hand_end < hands_content.length()) {
          if (hands_content[hand_end] == '{')
            brace_depth++;
          else if (hands_content[hand_end] == '}') {
            brace_depth--;
            if (brace_depth == 0)
              break;
          }
          hand_end++;
        }

        if (brace_depth != 0)
          break;

        std::string hand_obj_str =
            hands_content.substr(hand_start, hand_end - hand_start + 1);
        pos = hand_end + 1;
        parsed_hands++;

        Hand hand;
        hand.handedness = "Unknown";
        hand.confidence = 0.0f;

        // Parse handedness
        size_t hnd_pos = hand_obj_str.find("\"handedness\"");
        if (hnd_pos != std::string::npos) {
          size_t colon = hand_obj_str.find(":", hnd_pos);
          size_t quote1 = hand_obj_str.find("\"", colon);
          size_t quote2 = hand_obj_str.find("\"", quote1 + 1);
          if (quote1 != std::string::npos && quote2 != std::string::npos) {
            hand.handedness =
                hand_obj_str.substr(quote1 + 1, quote2 - quote1 - 1);
          }
        }

        // Parse confidence
        size_t conf_pos = hand_obj_str.find("\"confidence\"");
        if (conf_pos != std::string::npos) {
          size_t colon = hand_obj_str.find(":", conf_pos);
          size_t val_start = hand_obj_str.find_first_not_of(" \t", colon + 1);
          size_t val_end = hand_obj_str.find_first_of(",}", val_start);
          std::string conf_str =
              hand_obj_str.substr(val_start, val_end - val_start);
          try {
            hand.confidence = std::stof(conf_str);
          } catch (...) {
            hand.confidence = 0.0f;
          }
        }

        // Parse landmarks array
        size_t lm_key = hand_obj_str.find("\"landmarks\"");
        if (lm_key != std::string::npos) {
          size_t lm_array_open = hand_obj_str.find("[", lm_key);
          if (lm_array_open != std::string::npos) {
            // Find matching ]
            int lm_bracket = 0;
            size_t lm_array_close = lm_array_open;
            while (lm_array_close < hand_obj_str.length()) {
              if (hand_obj_str[lm_array_close] == '[')
                lm_bracket++;
              else if (hand_obj_str[lm_array_close] == ']') {
                lm_bracket--;
                if (lm_bracket == 0)
                  break;
              }
              lm_array_close++;
            }

            if (lm_bracket == 0) {
              std::string lm_content = hand_obj_str.substr(
                  lm_array_open + 1, lm_array_close - lm_array_open - 1);

              // Parse each landmark
              size_t lm_pos = 0;
              while (lm_pos < lm_content.length()) {
                // Skip whitespace
                while (
                    lm_pos < lm_content.length() &&
                    (lm_content[lm_pos] == ' ' || lm_content[lm_pos] == '\t' ||
                     lm_content[lm_pos] == '\n' || lm_content[lm_pos] == ',')) {
                  lm_pos++;
                }

                if (lm_pos >= lm_content.length() || lm_content[lm_pos] != '{')
                  break;

                // Find matching }
                int lm_brace = 0;
                size_t lm_start = lm_pos;
                size_t lm_end = lm_pos;
                while (lm_end < lm_content.length()) {
                  if (lm_content[lm_end] == '{')
                    lm_brace++;
                  else if (lm_content[lm_end] == '}') {
                    lm_brace--;
                    if (lm_brace == 0)
                      break;
                  }
                  lm_end++;
                }

                if (lm_brace != 0)
                  break;

                std::string lm_obj =
                    lm_content.substr(lm_start, lm_end - lm_start + 1);
                lm_pos = lm_end + 1;

                int x = 0, y = 0;

                // Parse x
                size_t x_pos = lm_obj.find("\"x\"");
                if (x_pos != std::string::npos) {
                  size_t colon = lm_obj.find(":", x_pos);
                  size_t val = lm_obj.find_first_not_of(" \t", colon + 1);
                  size_t end = lm_obj.find_first_of(",}", val);
                  try {
                    x = std::stoi(lm_obj.substr(val, end - val));
                  } catch (...) {
                  }
                }

                // Parse y
                size_t y_pos = lm_obj.find("\"y\"");
                if (y_pos != std::string::npos) {
                  size_t colon = lm_obj.find(":", y_pos);
                  size_t val = lm_obj.find_first_not_of(" \t", colon + 1);
                  size_t end = lm_obj.find_first_of(",}", val);
                  try {
                    y = std::stoi(lm_obj.substr(val, end - val));
                  } catch (...) {
                  }
                }

                hand.landmarks.push_back(cv::Point(x, y));
              }
            }
          }
        }

        if (!hand.landmarks.empty()) {
          hands.push_back(hand);
          if (frame_count == 1) {
            std::cerr << "Hand " << parsed_hands << ": " << hand.handedness
                      << ", " << hand.landmarks.size() << " landmarks\n";
          }
        }
      }

      if (frame_count == 1 || frame_count % 60 == 0) {
        std::cerr << "Frame " << frame_count << ": Found " << hands.size()
                  << " hands\n";
      }
    }
  } catch (const std::exception &e) {
    std::cerr << "Error in detectHands: " << e.what() << "\n";
  }

  return hands;
}
