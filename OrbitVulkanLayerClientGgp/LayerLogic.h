// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_
#define ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_

#include <string.h>

#include "LayerTimes.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

// Contains the logic of the OrbitVulkanLayerClientGgp, which keeps track of the calculates the FPS
// and run Orbit captures automatically when they drop below a certain threshold. It also
// instantiates the classes and variables needed for this so the layer itself is transparent to it.
class LayerLogic {
 public:
  LayerLogic() : data_initialised_{false}, orbit_capture_running_{false}, skip_logic_call_{true} {}

  void InitLayerData();
  void CleanLayerData();
  void ProcessQueuePresentKHR();

 private:
  bool data_initialised_;
  bool orbit_capture_running_;
  bool skip_logic_call_;
  std::unique_ptr<CaptureClientGgpClient> ggp_capture_client_;
  LayerTimes layer_times_;

  void StartOrbitCaptureService();
  void RunCapture();
  void StopCapture();
};

#endif  // ORBIT_VULKAN_LAYER_CLIENT_GGP_LAYER_LOGIC_H_
