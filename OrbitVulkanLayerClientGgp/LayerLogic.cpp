#include "LayerLogic.h"

#include <string.h>
#include <unistd.h>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"
#include "absl/base/casts.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"

namespace {
static constexpr uint16_t kGrpcPort = 44767;
static constexpr float_t kFrameTimeThreshold = 16.66;  // milliseconds
static constexpr uint32_t kCaptureLength = 10;         // seconds
}  // namespace

void LayerLogic::StartOrbitCaptureService() {
  LOG("Starting Orbit capture service");
  pid_t pid = fork();
  if (pid < 0) {
    ERROR("Fork failed; not able to start the capture service");
  } else if (pid == 0) {
    std::string game_pid = absl::StrFormat("%d", getppid());
    // TODO(crisguerrero): Read the arguments from a config file.
    char* argv[] = {strdup("/mnt/developer/OrbitCaptureGgpService"),
                    strdup("-pid"),
                    game_pid.data(),
                    strdup("-log_directory"),
                    strdup("/var/game/"),
                    NULL};

    LOG("Making call to %s %s %s %s %s", argv[0], argv[1], argv[2], argv[3], argv[4]);
    execv(argv[0], argv);
  }
}

void LayerLogic::InitLayerData() {
  // Although this method is expected to be called just once, we include a flag to make sure the
  // gRPC service and client are not initialised more than once.
  if (data_initialised_ == false) {
    LOG("Making initialisations required in the layer");

    // Start the orbit capture service in a new thread.
    StartOrbitCaptureService();

    // Initialise the client and establish the channel to make calls to the service.
    std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", kGrpcPort);
    ggp_capture_client_ =
        std::unique_ptr<CaptureClientGgpClient>(new CaptureClientGgpClient(grpc_server_address));

    data_initialised_ = true;
  }
}

void LayerLogic::CleanLayerData() {
  if (data_initialised_ == true) {
    ggp_capture_client_->ShutdownService();
    data_initialised_ = false;
    orbit_capture_running_ = false;
    skip_logic_call_ = true;
  }
}

// QueuePresentKHR is called once per frame so we can calculate the time per frame. When this value
// is higher than a certain threshold, an Orbit capture is started and runs during a certain period
// of time; after which is stopped and saved.
void LayerLogic::ProcessQueuePresentKHR() {
  std::chrono::steady_clock::time_point current_frame = std::chrono::steady_clock::now();
  // Ignore logic on the first callbecause times are not initialised. Also skipped right after a
  // capture has been stopped
  if (skip_logic_call_ == false) {
    if (orbit_capture_running_ == false) {
      auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(
          current_frame - layer_times_.last_frame);
      // LOG("Frame time: %.2f ms", frame_time.count());
      if (frame_time.count() > kFrameTimeThreshold) {
        RunCapture();
        LOG("Time frame is %.2fms and exceeds the %.2fms threshold; starting capture",
            frame_time.count(), kFrameTimeThreshold);
      }
    } else {
      // Stop capture if it has been running enough time
      auto capture_time = std::chrono::duration_cast<std::chrono::seconds>(
          current_frame - layer_times_.capture_started);
      // LOG("Capture time: %.2f s", capture_time.count());
      if (capture_time.count() >= kCaptureLength) {
        LOG("Capture has been running for %ds; stopping it", kCaptureLength);
        StopCapture();
      }
    }
  } else {
    skip_logic_call_ = false;
  }
  layer_times_.last_frame = current_frame;
}

void LayerLogic::RunCapture() {
  int capture_started = ggp_capture_client_->StartCapture();
  if (capture_started == 1) {
    layer_times_.capture_started = std::chrono::steady_clock::now();
    orbit_capture_running_ = true;
  }
}

void LayerLogic::StopCapture() {
  int capture_stopped = ggp_capture_client_->StopAndSaveCapture();
  if (capture_stopped == 1) {
    orbit_capture_running_ = false;
    // The frame time is expected to be longer the next call so we skip the check
    skip_logic_call_ = true;
  }
}
