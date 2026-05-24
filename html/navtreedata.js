/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Group 31 Project", "index.html", [
    [ "Smart Gesture Controls - Hand Tracking Documentation", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html", [
      [ "Overview", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md1", null ],
      [ "Architecture", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md3", null ],
      [ "Components", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md5", [
        [ "1. <b>main.cpp</b> - Main Application Loop", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md6", null ],
        [ "2. <b>HandTracker (C++)</b> - Python Process Manager", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md8", [
          [ "<b>HandTracker Class</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md9", null ],
          [ "<b>Hand Structure</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md10", null ],
          [ "<b>Communication Protocol</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md11", null ],
          [ "<b>JSON Parsing</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md12", null ]
        ] ],
        [ "3. <b>hand_detector.py</b> - MediaPipe Inference", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md14", [
          [ "<b>MediaPipe Hand Landmarks</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md15", null ],
          [ "<b>HandDetector Class</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md16", null ],
          [ "<b>Detection Pipeline</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md17", null ]
        ] ]
      ] ],
      [ "Hand Landmark Skeleton Connections", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md19", null ],
      [ "File Structure", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md21", null ],
      [ "Building and Running", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md23", [
        [ "Prerequisites", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md24", null ],
        [ "Build Steps", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md25", null ],
        [ "Controls", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md26", null ]
      ] ],
      [ "Performance Optimization Tips", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md28", [
        [ "1. <b>Reduce Frame Processing</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md29", null ],
        [ "2. <b>Lower Camera Resolution</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md30", null ],
        [ "3. <b>Adjust JPEG Quality</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md31", null ],
        [ "4. <b>Increase Detection Confidence</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md32", null ]
      ] ],
      [ "Adding Support for More Hands", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md34", [
        [ "Python Side (hand_detector.py)", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md35", null ],
        [ "C++ Side (main.cpp)", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md36", null ]
      ] ],
      [ "UI Customization", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md38", [
        [ "Color Scheme", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md39", null ],
        [ "Font and Text Size", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md40", null ],
        [ "Landmark Size", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md41", null ]
      ] ],
      [ "Troubleshooting", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md43", [
        [ "\"HandTracker initialized successfully\" but no landmarks", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md44", null ],
        [ "Lag/Stuttering", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md45", null ],
        [ "\"21 hands detected\" with only 1-2 hands", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md46", null ]
      ] ],
      [ "Future Enhancements", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md48", [
        [ "1. <b>Gesture Recognition</b> (GestureClassifier.cpp)", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md49", null ],
        [ "2. <b>Action Mapping</b> (ActionMapper.cpp)", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md50", null ],
        [ "3. <b>Multi-threading</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md51", null ],
        [ "4. <b>Save/Load Configurations</b>", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md52", null ]
      ] ],
      [ "Technical Details", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md54", [
        [ "Base64 Encoding", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md55", null ],
        [ "JSON Parsing", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md56", null ]
      ] ],
      [ "Common Issues and Solutions", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md58", null ],
      [ "References", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md60", null ],
      [ "License", "md__d_o_c_u_m_e_n_t_a_t_i_o_n.html#autotoc_md62", null ]
    ] ],
    [ "Smart Gesture Controls - Hand Tracking", "md__r_e_a_d_m_e.html", [
      [ "Quick Start", "md__r_e_a_d_m_e.html#autotoc_md64", null ],
      [ "Features", "md__r_e_a_d_m_e.html#autotoc_md65", null ],
      [ "Architecture", "md__r_e_a_d_m_e.html#autotoc_md66", null ],
      [ "Project Structure", "md__r_e_a_d_m_e.html#autotoc_md67", null ],
      [ "Hand Landmarks", "md__r_e_a_d_m_e.html#autotoc_md68", null ],
      [ "Customization", "md__r_e_a_d_m_e.html#autotoc_md69", [
        [ "Change Number of Hands", "md__r_e_a_d_m_e.html#autotoc_md70", null ],
        [ "Adjust Colors", "md__r_e_a_d_m_e.html#autotoc_md71", null ],
        [ "Change Frame Rate", "md__r_e_a_d_m_e.html#autotoc_md72", null ]
      ] ],
      [ "Performance Tips", "md__r_e_a_d_m_e.html#autotoc_md73", null ],
      [ "Documentation", "md__r_e_a_d_m_e.html#autotoc_md74", null ],
      [ "Dependencies", "md__r_e_a_d_m_e.html#autotoc_md75", null ],
      [ "Building from Scratch", "md__r_e_a_d_m_e.html#autotoc_md76", [
        [ "macOS", "md__r_e_a_d_m_e.html#autotoc_md77", null ],
        [ "Windows", "md__r_e_a_d_m_e.html#autotoc_md78", null ]
      ] ],
      [ "Troubleshooting", "md__r_e_a_d_m_e.html#autotoc_md79", null ],
      [ "Future Enhancements", "md__r_e_a_d_m_e.html#autotoc_md80", null ],
      [ "License", "md__r_e_a_d_m_e.html#autotoc_md81", null ],
      [ "Original Project Goals", "md__r_e_a_d_m_e.html#autotoc_md82", null ],
      [ "Author", "md__r_e_a_d_m_e.html#autotoc_md83", null ]
    ] ],
    [ "Smart Gesture Controls", "md__r_e_a_d_m_e_8old.html", [
      [ "Overview", "md__r_e_a_d_m_e_8old.html#autotoc_md86", null ],
      [ "Features", "md__r_e_a_d_m_e_8old.html#autotoc_md88", [
        [ "Example Gestures", "md__r_e_a_d_m_e_8old.html#autotoc_md89", null ]
      ] ],
      [ "Project Structure", "md__r_e_a_d_m_e_8old.html#autotoc_md91", null ],
      [ "Requirements", "md__r_e_a_d_m_e_8old.html#autotoc_md93", null ],
      [ "Build Instructions", "md__r_e_a_d_m_e_8old.html#autotoc_md95", [
        [ "macOS", "md__r_e_a_d_m_e_8old.html#autotoc_md96", null ],
        [ "Windows", "md__r_e_a_d_m_e_8old.html#autotoc_md97", null ]
      ] ],
      [ "Build &amp; Run", "md__r_e_a_d_m_e_8old.html#autotoc_md99", null ],
      [ "Development Workflow", "md__r_e_a_d_m_e_8old.html#autotoc_md101", null ],
      [ "Current Status", "md__r_e_a_d_m_e_8old.html#autotoc_md103", null ],
      [ "Notes", "md__r_e_a_d_m_e_8old.html#autotoc_md105", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';
var LISTOFALLMEMBERS = 'List of all members';