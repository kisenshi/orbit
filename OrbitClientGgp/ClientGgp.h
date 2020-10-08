// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_GGP_CLIENT_GGP_H_
#define ORBIT_CLIENT_GGP_CLIENT_GGP_H_

#include <cstdint>
#include <memory>
#include <string>

#include "CaptureData.h"
#include "ClientGgpOptions.h"
#include "ClientGgpTimes.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureClient/CaptureClient.h"
#include "OrbitCaptureClient/CaptureListener.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientServices/ProcessClient.h"
#include "StringManager.h"
#include "TracepointCustom.h"
#include "grpcpp/grpcpp.h"

class ClientGgp final : public CaptureListener {
 public:
  ClientGgp(ClientGgpOptions&& options, ClientGgpTimes times);
  bool InitClient();
  bool RequestStartCapture(ThreadPool* thread_pool);
  bool StopCapture();
  bool SaveCapture();
  void LogTimes();

  // CaptureListener implementation
  void OnCaptureStarted(
      ProcessData&& process, absl::flat_hash_map<std::string, ModuleData*>&& module_map,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      TracepointInfoSet selected_tracepoints) override;
  void OnCaptureComplete() override;
  void OnCaptureCancelled() override;
  void OnCaptureFailed(ErrorMessage error_message) override;
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  void OnKeyAndString(uint64_t key, std::string str) override;
  void OnUniqueCallStack(CallStack callstack) override;
  void OnCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) override;
  void OnThreadName(int32_t thread_id, std::string thread_name) override;
  void OnAddressInfo(orbit_client_protos::LinuxAddressInfo address_info) override;
  void OnUniqueTracepointInfo(uint64_t key,
                              orbit_grpc_protos::TracepointInfo tracepoint_info) override;
  void OnTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) override;

 private:
  ClientGgpOptions options_;
  ClientGgpTimes capture_times_;
  std::shared_ptr<grpc::Channel> grpc_channel_;
  ProcessData target_process_;
  absl::flat_hash_set<std::unique_ptr<ModuleData>> modules_;
  absl::flat_hash_map<std::string, ModuleData*> module_map_;
  ModuleData* main_module_;
  std::shared_ptr<StringManager> string_manager_;
  std::unique_ptr<CaptureClient> capture_client_;
  std::unique_ptr<ProcessClient> process_client_;
  CaptureData capture_data_;
  std::vector<orbit_client_protos::TimerInfo> timer_infos_;

  ErrorMessageOr<ProcessData> GetOrbitProcessByPid(int32_t pid);
  bool InitCapture();
  ErrorMessageOr<void> LoadModuleAndSymbols();
  std::string SelectedFunctionMatch(const orbit_client_protos::FunctionInfo& func);
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> GetSelectedFunctions();
  void InformUsedSelectedCaptureFunctions(
      const absl::flat_hash_set<std::string>& capture_functions_used);
  void ProcessTimer(const orbit_client_protos::TimerInfo& timer_info);
};

#endif  // ORBIT_CLIENT_GGP_CLIENT_GGP_H_