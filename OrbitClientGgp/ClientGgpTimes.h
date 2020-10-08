// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_TIMES_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_TIMES_H_

#include <chrono>

struct ClientGgpTimes {
  std::chrono::steady_clock::time_point start_ggp_client;
  std::chrono::steady_clock::time_point client_initialised;
  std::chrono::steady_clock::time_point start_capture_requested;
  std::chrono::steady_clock::time_point capture_requested;
  std::chrono::steady_clock::time_point capture_started;
};

#endif  // ORBIT_CLIENT_GGP_CLIENT_GGP_TIMES_H_