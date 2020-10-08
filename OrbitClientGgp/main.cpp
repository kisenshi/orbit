// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>
#include <vector>

#include "ClientGgp.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitVersion/OrbitVersion.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(uint32_t, capture_length, 10, "duration of capture in seconds");
ABSL_FLAG(std::vector<std::string>, functions, {},
          "Comma-separated list of functions to hook to the capture");
ABSL_FLAG(std::string, file_name, "", "File name used for saving the capture");
ABSL_FLAG(std::string, file_directory, "/var/game/",
          "Path to locate orbit file. By default it is /var/game/");
ABSL_FLAG(std::string, log_directory, "",
          "Path to locate debug file. By default only stdout is used for logs");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

namespace {

std::string GetLogFilePath(const std::string& log_directory) {
  std::filesystem::path log_directory_path{log_directory};
  std::filesystem::create_directory(log_directory_path);
  const std::string log_file_path = log_directory_path / "OrbitClientGgp.log";
  LOG("Log file: %s", log_file_path);
  return log_file_path;
}

}  // namespace

int main(int argc, char** argv) {
  ClientGgpTimes client_times;
  client_times.start_ggp_client = std::chrono::steady_clock::now();
  absl::SetProgramUsageMessage("Orbit CPU Profiler Ggp Client");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &OrbitCore::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  const std::string log_directory = absl::GetFlag(FLAGS_log_directory);
  if (!log_directory.empty()) {
    InitLogFile(GetLogFilePath(log_directory));
  }

  if (!absl::GetFlag(FLAGS_pid)) {
    FATAL("pid to capture not provided; set using -pid");
  }

  ClientGgpOptions options;
  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  options.capture_pid = absl::GetFlag(FLAGS_pid);
  options.capture_functions = absl::GetFlag(FLAGS_functions);
  options.capture_file_name = absl::GetFlag(FLAGS_file_name);
  options.capture_file_directory = absl::GetFlag(FLAGS_file_directory);

  ClientGgp client_ggp(std::move(options), client_times);
  if (!client_ggp.InitClient()) {
    return -1;
  }

  // The request is done in a separate thread to avoid blocking main()
  // It is needed to provide a thread pool
  std::unique_ptr<ThreadPool> thread_pool = ThreadPool::Create(1, 1, absl::Seconds(1));
  if (!client_ggp.RequestStartCapture(thread_pool.get())) {
    thread_pool->ShutdownAndWait();
    FATAL("Not possible to start the capture; exiting program");
  }

  // Captures for the period of time requested
  uint32_t capture_length = absl::GetFlag(FLAGS_capture_length);
  LOG("Go to sleep for %d seconds", capture_length);
  absl::SleepFor(absl::Seconds(capture_length));
  LOG("Back from sleep");

  // Requests to stop the capture and waits for thread to finish
  if (!client_ggp.StopCapture()) {
    thread_pool->ShutdownAndWait();
    FATAL("Not possible to stop the capture; exiting program");
  }
  LOG("Shut down the thread and wait for it to finish");
  thread_pool->ShutdownAndWait();

  if (!client_ggp.SaveCapture()) {
    return -1;
  }

  client_ggp.LogTimes();

  LOG("All done");
  return 0;
}