#include "HandTracker.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

// ── Platform-specific includes ────────────────────────────────────────────────
#ifdef _WIN32
// Windows.h already pulled in via HandTracker.hpp
#  include <io.h>       // _open_osfhandle
#  include <fcntl.h>    // _O_RDONLY
#  define sleep_ms(ms) Sleep(ms)
#else
#  include <unistd.h>
#  include <sys/wait.h>
#  include <signal.h>
#  define sleep_ms(ms) usleep((ms) * 1000)
#endif

#ifdef __APPLE__
#  include <mach-o/dyld.h>  // _NSGetExecutablePath
#endif

// ── getProjectRoot ────────────────────────────────────────────────────────────
// Returns the project root directory at runtime.
// Assumes the executable lives in <root>/build/, so the root is one level up.
//
// Fixes vs. previous version:
//   • lastSlash is now declared BEFORE the #ifdef block with a safe default.
//     Previously it was declared inside each branch but used after #endif,
//     which compilers warn about as "potentially uninitialized".
//   • All buf[] arrays are zero-initialised with = {} so garbage bytes are
//     never read if the OS call partially fails.
//   • Linux branch only assigns exePath when readlink() actually succeeds.
//   • Windows now uses GetModuleFileNameW + WideCharToMultiByte instead of
//     GetModuleFileNameA, which is undefined in some SDK/compiler combinations
//     (e.g. when UNICODE is defined, or with certain MinGW configurations).
static std::string getProjectRoot() {
    // Step 1: resolve the directory containing this executable.
    std::string exeDir;

#ifdef _WIN32
    wchar_t wbuf[MAX_PATH] = {};
    GetModuleFileNameW(NULL, wbuf, MAX_PATH);
    char buf[MAX_PATH * 4] = {};
    WideCharToMultiByte(CP_UTF8, 0, wbuf, -1,
        buf, static_cast<int>(sizeof(buf)), NULL, NULL);
    std::string exePath(buf);
    size_t slash = exePath.find_last_of('\\');
    exeDir = (slash != std::string::npos) ? exePath.substr(0, slash) : ".";
    const char SEP = '\\';

#elif defined(__APPLE__)
    char     buf[1024] = {};
    uint32_t sz = static_cast<uint32_t>(sizeof(buf));
    _NSGetExecutablePath(buf, &sz);
    std::string exePath(buf);
    size_t slash = exePath.find_last_of('/');
    exeDir = (slash != std::string::npos) ? exePath.substr(0, slash) : ".";
    const char SEP = '/';

#else  // Linux
    char    buf[1024] = {};
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) buf[len] = '\0';
    std::string exePath(buf);
    size_t slash = exePath.find_last_of('/');
    exeDir = (slash != std::string::npos) ? exePath.substr(0, slash) : ".";
    const char SEP = '/';
#endif

    // Step 2: walk upward from exeDir until we find CMakeLists.txt.
    // This handles any build layout (build/, build/Debug/, build/x64/Debug/, etc.)
    // without hardcoding the number of levels to ascend.
    std::string candidate = exeDir;
    for (int depth = 0; depth < 8; ++depth) {
        std::string landmark = candidate + SEP + "CMakeLists.txt";

#ifdef _WIN32
        if (GetFileAttributesA(landmark.c_str()) != INVALID_FILE_ATTRIBUTES)
            return candidate;                          // found the project root
#else
        if (access(landmark.c_str(), F_OK) == 0)
            return candidate;
#endif
        // Go one level up: append /.. (or \..)
        candidate += SEP;
        candidate += "..";
    }

    // Fallback: if CMakeLists.txt was never found just return one level up,
    // which is the original behaviour, and let the path diagnostics explain.
    std::cerr << "[HandTracker] WARNING: CMakeLists.txt not found while walking up "
        << "from " << exeDir << ". Falling back to one level up.\n";
    return exeDir + SEP + "..";
}

// ── base64_encode ─────────────────────────────────────────────────────────────
// Fix: char_array_3 and char_array_4 were uninitialised. In the tail path
// (partial 3-byte group) char_array_4[3] was read without ever being written
// when i == 1. Zero-initialising both at declaration is the simplest fix.
static const char base64_chars[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64_encode(const unsigned char* buf, unsigned int buflen) {
    std::string   ret;
    int           i = 0;
    unsigned char char_array_3[3] = {}; // zero-init
    unsigned char char_array_4[4] = {}; // zero-init

    while (buflen--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) |
                ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) |
                ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for (i = 0; i < 4; i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i > 0) {
        for (int j = i; j < 3; j++) char_array_3[j] = '\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) |
            ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) |
            ((char_array_3[2] & 0xc0) >> 6);
        for (int j = 0; j <= i; j++) ret += base64_chars[char_array_4[j]];
        while (i++ < 3) ret += '=';
    }

    return ret;
}

// ── Constructor / Destructor ──────────────────────────────────────────────────
HandTracker::HandTracker()
    : initialized_(false),
    python_read_(nullptr),
    python_write_(nullptr)
#ifdef _WIN32
    , child_process_handle_(INVALID_HANDLE_VALUE)
#else
    , child_pid_(-1)
#endif
{
    if (startPythonProcess()) {
        initialized_ = true;
        std::cout << "HandTracker initialized successfully\n";
    }
    else {
        std::cerr << "Error initializing HandTracker\n";
    }
}

HandTracker::~HandTracker() { stopPythonProcess(); }

// ── startPythonProcess ────────────────────────────────────────────────────────
#ifdef _WIN32
// ── Windows ───────────────────────────────────────────────────────────────────
bool HandTracker::startPythonProcess() {
    std::string root = getProjectRoot();
    std::string python_path = root + "\\.venv\\Scripts\\python.exe";
    std::string script_path = root + "\\hand_detector.py";
    std::string cmd = "\"" + python_path + "\" \"" + script_path + "\"";

    // Always print resolved paths so misconfiguration is immediately visible
    std::cerr << "[HandTracker] Project root : " << root << "\n"
        << "[HandTracker] Python path  : " << python_path << "\n"
        << "[HandTracker] Script path  : " << script_path << "\n"
        << "[HandTracker] Command      : " << cmd << "\n";

    // Validate both files exist before attempting CreateProcess.
    // GetFileAttributesA returns INVALID_FILE_ATTRIBUTES when the path is wrong.
    if (GetFileAttributesA(python_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[HandTracker] ERROR: Python executable not found.\n"
            << "              Expected: " << python_path << "\n"
            << "              Fix: run 'python -m venv .venv' from the project root.\n";
        return false;
    }
    if (GetFileAttributesA(script_path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::cerr << "[HandTracker] ERROR: hand_detector.py not found.\n"
            << "              Expected: " << script_path << "\n"
            << "              Fix: make sure hand_detector.py is in the project root, not in build/.\n";
        return false;
    }

    // All child handles must be inheritable
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Pipe: parent writes → child reads (child's stdin)
    HANDLE hStdinRd = NULL;
    HANDLE hStdinWr = NULL;
    if (!CreatePipe(&hStdinRd, &hStdinWr, &sa, 0)) {
        std::cerr << "CreatePipe (stdin) failed: " << GetLastError() << "\n";
        return false;
    }
    SetHandleInformation(hStdinWr, HANDLE_FLAG_INHERIT, 0); // parent write end: not inherited

    // Pipe: child writes → parent reads (child's stdout)
    HANDLE hStdoutRd = NULL;
    HANDLE hStdoutWr = NULL;
    if (!CreatePipe(&hStdoutRd, &hStdoutWr, &sa, 0)) {
        std::cerr << "CreatePipe (stdout) failed: " << GetLastError() << "\n";
        CloseHandle(hStdinRd);
        CloseHandle(hStdinWr);
        return false;
    }
    SetHandleInformation(hStdoutRd, HANDLE_FLAG_INHERIT, 0); // parent read end: not inherited

    STARTUPINFOA si{};
    si.cb = sizeof(STARTUPINFOA);
    si.hStdInput = hStdinRd;
    si.hStdOutput = hStdoutWr;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE); // child stderr stays on terminal
    si.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(
        NULL,
        &cmd[0],  // CreateProcessA requires a mutable buffer
        NULL, NULL,
        TRUE,     // inherit handles
        0, NULL, NULL,
        &si, &pi
    );

    // Close the child-side handle copies we no longer need in the parent
    CloseHandle(hStdinRd);
    CloseHandle(hStdoutWr);

    if (!ok) {
        DWORD err = GetLastError();
        std::cerr << "[HandTracker] CreateProcess failed (error " << err << "): ";
        switch (err) {
        case 2:   std::cerr << "File not found — paths above are wrong.\n";             break;
        case 3:   std::cerr << "Path not found — a directory in the path is missing.\n"; break;
        case 5:   std::cerr << "Access denied — check file permissions.\n";              break;
        case 193: std::cerr << "Not a valid Win32 app — is the .venv corrupted?\n";     break;
        default:  std::cerr << "Unknown error.\n";                                        break;
        }
        CloseHandle(hStdinWr);
        CloseHandle(hStdoutRd);
        return false;
    }

    CloseHandle(pi.hThread);
    child_process_handle_ = pi.hProcess;

    // Wrap the raw Win32 handles in FILE* for portable fgets/fwrite usage
    int write_fd = _open_osfhandle(reinterpret_cast<intptr_t>(hStdinWr), 0);
    int read_fd = _open_osfhandle(reinterpret_cast<intptr_t>(hStdoutRd), _O_RDONLY);
    if (write_fd == -1 || read_fd == -1) {
        std::cerr << "_open_osfhandle failed\n";
        return false;
    }

    python_write_ = _fdopen(write_fd, "w");
    python_read_ = _fdopen(read_fd, "r");
    if (!python_write_ || !python_read_) {
        std::cerr << "_fdopen failed\n";
        return false;
    }

    // Wait up to 3 seconds for the Python script to emit "READY"
    char buffer[256] = {};
    for (int attempts = 0; attempts < 30; ++attempts) {
        if (fgets(buffer, sizeof(buffer), python_read_) != nullptr) {
            std::string response(buffer);
            std::cout << "Python response: " << response;
            if (response.find("READY") != std::string::npos) return true;
            if (response.find("FAILED") != std::string::npos) return false;
        }
        sleep_ms(100);
    }
    std::cerr << "Timed out waiting for Python READY signal\n";
    return false;
}

#else
// ── macOS / Linux ─────────────────────────────────────────────────────────────
bool HandTracker::startPythonProcess() {
    std::string root = getProjectRoot();
    std::string python_path = root + "/.venv/bin/python3";
    std::string script_path = root + "/hand_detector.py";

    // to_child[0]   = read end  — child reads its stdin from here
    // to_child[1]   = write end — parent writes frames here
    // from_child[0] = read end  — parent reads JSON from here
    // from_child[1] = write end — child writes its stdout here
    int to_child[2] = { -1, -1 };
    int from_child[2] = { -1, -1 };

    if (pipe(to_child) == -1 || pipe(from_child) == -1) {
        std::cerr << "pipe() failed\n";
        return false;
    }

    child_pid_ = fork();
    if (child_pid_ < 0) {
        std::cerr << "fork() failed\n";
        close(to_child[0]);   close(to_child[1]);
        close(from_child[0]); close(from_child[1]);
        return false;
    }

    if (child_pid_ == 0) {
        // ── Child process ──────────────────────────────────────────────────
        dup2(to_child[0], STDIN_FILENO);   // child stdin  ← parent writes
        dup2(from_child[1], STDOUT_FILENO);  // child stdout → parent reads
        // stderr is left on the terminal for diagnostics

        close(to_child[0]);   close(to_child[1]);
        close(from_child[0]); close(from_child[1]);

        execl(python_path.c_str(),
            python_path.c_str(),
            script_path.c_str(),
            static_cast<char*>(nullptr));

        std::cerr << "execl() failed\n";
        _exit(1);
    }

    // ── Parent process ─────────────────────────────────────────────────────
    close(to_child[0]);   // child's read end — parent doesn't use it
    close(from_child[1]); // child's write end — parent doesn't use it

    python_write_ = fdopen(to_child[1], "w");
    python_read_ = fdopen(from_child[0], "r");
    if (!python_write_ || !python_read_) {
        std::cerr << "fdopen() failed\n";
        return false;
    }

    // Wait up to 3 seconds for the Python script to emit "READY"
    char buffer[256] = {};
    for (int attempts = 0; attempts < 30; ++attempts) {
        if (fgets(buffer, sizeof(buffer), python_read_) != nullptr) {
            std::string response(buffer);
            std::cout << "Python response: " << response;
            if (response.find("READY") != std::string::npos) return true;
            if (response.find("FAILED") != std::string::npos) return false;
        }
        sleep_ms(100);
    }
    std::cerr << "Timed out waiting for Python READY signal\n";
    return false;
}
#endif  // _WIN32

// ── stopPythonProcess ─────────────────────────────────────────────────────────
void HandTracker::stopPythonProcess() {
    // Close the write end first — child sees EOF on its stdin and can exit cleanly
    if (python_write_) { fclose(python_write_); python_write_ = nullptr; }
    if (python_read_) { fclose(python_read_);  python_read_ = nullptr; }

#ifdef _WIN32
    if (child_process_handle_ != INVALID_HANDLE_VALUE) {
        if (WaitForSingleObject(child_process_handle_, 3000) != WAIT_OBJECT_0)
            TerminateProcess(child_process_handle_, 0);
        CloseHandle(child_process_handle_);
        child_process_handle_ = INVALID_HANDLE_VALUE;
    }
#else
    if (child_pid_ > 0) {
        kill(child_pid_, SIGTERM);
        waitpid(child_pid_, nullptr, 0); // prevent zombie
        child_pid_ = -1;
    }
#endif
}

// ── detectHands ───────────────────────────────────────────────────────────────
std::vector<Hand> HandTracker::detectHands(const cv::Mat& frame) {
    std::vector<Hand> hands;

    if (!initialized_ || !python_read_ || !python_write_)
        return hands;

    static int frame_count = 0;
    ++frame_count;

    try {
        // Encode frame to JPEG, then to base64
        std::vector<uchar> buf;
        cv::imencode(".jpg", frame, buf);
        std::string b64_data =
            base64_encode(buf.data(), static_cast<unsigned int>(buf.size()));

        // Protocol: "<byte_count>\n<base64_payload>\n"
        std::string size_line = std::to_string(b64_data.size()) + "\n";
        fwrite(size_line.c_str(), 1, size_line.size(), python_write_);
        fwrite(b64_data.c_str(), 1, b64_data.size(), python_write_);
        fwrite("\n", 1, 1, python_write_);
        fflush(python_write_);

        // Read one line of JSON back from Python
        char buffer[65536] = {};
        if (fgets(buffer, sizeof(buffer), python_read_) == nullptr)
            return hands;

        std::string json_str(buffer);

        if (frame_count == 1 || frame_count % 60 == 0) {
            std::string preview =
                json_str.substr(0, std::min(size_t(300), json_str.length()));
            std::cerr << "Frame " << frame_count
                << " JSON preview: " << preview << "\n";
        }

        // ── Locate the "hands" array ──────────────────────────────────────────
        size_t hands_key = json_str.find("\"hands\"");
        if (hands_key == std::string::npos) return hands;

        size_t array_open = json_str.find('[', hands_key);
        if (array_open == std::string::npos) return hands;

        int    bracket_depth = 0;
        size_t array_close = array_open;
        while (array_close < json_str.length()) {
            if (json_str[array_close] == '[') ++bracket_depth;
            else if (json_str[array_close] == ']') { if (--bracket_depth == 0) break; }
            ++array_close;
        }
        if (bracket_depth != 0) return hands;

        std::string hands_content =
            json_str.substr(array_open + 1, array_close - array_open - 1);

        // ── Parse each hand object ────────────────────────────────────────────
        size_t pos = 0;
        int    parsed_hands = 0;

        while (pos < hands_content.length()) {
            while (pos < hands_content.length() &&
                (hands_content[pos] == ' ' || hands_content[pos] == '\t' ||
                    hands_content[pos] == '\n' || hands_content[pos] == ','))
                ++pos;

            if (pos >= hands_content.length() || hands_content[pos] != '{') break;

            int    brace_depth = 0;
            size_t hand_start = pos;
            size_t hand_end = pos;
            while (hand_end < hands_content.length()) {
                if (hands_content[hand_end] == '{') ++brace_depth;
                else if (hands_content[hand_end] == '}') { if (--brace_depth == 0) break; }
                ++hand_end;
            }
            if (brace_depth != 0) break;

            std::string hand_obj =
                hands_content.substr(hand_start, hand_end - hand_start + 1);
            pos = hand_end + 1;
            ++parsed_hands;

            Hand hand;
            hand.handedness = "Unknown";
            hand.confidence = 0.0f;

            // -- handedness --
            size_t hnd_pos = hand_obj.find("\"handedness\"");
            if (hnd_pos != std::string::npos) {
                size_t colon = hand_obj.find(':', hnd_pos);
                size_t quote1 = hand_obj.find('"', colon);
                size_t quote2 = hand_obj.find('"', quote1 + 1);
                if (quote1 != std::string::npos && quote2 != std::string::npos)
                    hand.handedness = hand_obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }

            // -- confidence --
            size_t conf_pos = hand_obj.find("\"confidence\"");
            if (conf_pos != std::string::npos) {
                size_t colon = hand_obj.find(':', conf_pos);
                size_t val_start = hand_obj.find_first_not_of(" \t", colon + 1);
                size_t val_end = hand_obj.find_first_of(",}", val_start);
                try {
                    hand.confidence =
                        std::stof(hand_obj.substr(val_start, val_end - val_start));
                }
                catch (...) {}
            }

            // -- landmarks --
            size_t lm_key = hand_obj.find("\"landmarks\"");
            if (lm_key != std::string::npos) {
                size_t lm_open = hand_obj.find('[', lm_key);
                if (lm_open != std::string::npos) {
                    int    lm_bracket = 0;
                    size_t lm_close = lm_open;
                    while (lm_close < hand_obj.length()) {
                        if (hand_obj[lm_close] == '[') ++lm_bracket;
                        else if (hand_obj[lm_close] == ']') { if (--lm_bracket == 0) break; }
                        ++lm_close;
                    }

                    if (lm_bracket == 0) {
                        std::string lm_content =
                            hand_obj.substr(lm_open + 1, lm_close - lm_open - 1);

                        size_t lm_pos = 0;
                        while (lm_pos < lm_content.length()) {
                            while (lm_pos < lm_content.length() &&
                                (lm_content[lm_pos] == ' ' ||
                                    lm_content[lm_pos] == '\t' ||
                                    lm_content[lm_pos] == '\n' ||
                                    lm_content[lm_pos] == ','))
                                ++lm_pos;

                            if (lm_pos >= lm_content.length() ||
                                lm_content[lm_pos] != '{') break;

                            int    lm_brace = 0;
                            size_t lm_start = lm_pos;
                            size_t lm_end = lm_pos;
                            while (lm_end < lm_content.length()) {
                                if (lm_content[lm_end] == '{') ++lm_brace;
                                else if (lm_content[lm_end] == '}') { if (--lm_brace == 0) break; }
                                ++lm_end;
                            }
                            if (lm_brace != 0) break;

                            std::string lm_obj =
                                lm_content.substr(lm_start, lm_end - lm_start + 1);
                            lm_pos = lm_end + 1;

                            int x = 0, y = 0;

                            size_t x_pos = lm_obj.find("\"x\"");
                            if (x_pos != std::string::npos) {
                                size_t colon = lm_obj.find(':', x_pos);
                                size_t val = lm_obj.find_first_not_of(" \t", colon + 1);
                                size_t end = lm_obj.find_first_of(",}", val);
                                try { x = std::stoi(lm_obj.substr(val, end - val)); }
                                catch (...) {}
                            }

                            size_t y_pos = lm_obj.find("\"y\"");
                            if (y_pos != std::string::npos) {
                                size_t colon = lm_obj.find(':', y_pos);
                                size_t val = lm_obj.find_first_not_of(" \t", colon + 1);
                                size_t end = lm_obj.find_first_of(",}", val);
                                try { y = std::stoi(lm_obj.substr(val, end - val)); }
                                catch (...) {}
                            }

                            hand.landmarks.push_back(cv::Point(x, y));
                        }
                    }
                }
            }

            if (!hand.landmarks.empty()) {
                hands.push_back(hand);
                if (frame_count == 1)
                    std::cerr << "Hand " << parsed_hands << ": "
                    << hand.handedness << ", "
                    << hand.landmarks.size() << " landmarks\n";
            }
        }

        if (frame_count == 1 || frame_count % 60 == 0)
            std::cerr << "Frame " << frame_count
            << ": Found " << hands.size() << " hands\n";

    }
    catch (const std::exception& e) {
        std::cerr << "Error in detectHands: " << e.what() << "\n";
    }

    return hands;
}