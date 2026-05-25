/*
author : irala rushendra
email  : iralarushendra@gmail.com
*/

#pragma once
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <memory>

// The following struct is returned to JS through JSI
struct AuthResult {
  bool isLive;
  float confidenceScore;
  std::string userId;
  long processingTimeMs;
};

class GhostSyncEngine {
  public:
  GhostSyncEngine();
  ~GhostSyncEngine();

  // The following constructor snippets makes sure that an instance cannot be copied or re-assigned
  GhostSyncEngine(const GhostSyncEngine&) = delete;
  GhostSyncEngine& operator=(const GhostSyncEngine&) = delete;


  // pushFrame(...) is called synchronously by JS via JSI whenever the camera provides a frame
  // Runs as a "Producer Thread"
  void pushFrame(const std::vector<uint8_t>& frameData, int width, int height);

  AuthResult getLatestResult();

  private:

  // processLoop(...) runs as a dedicated background thread for NCNN interface
  // Runs as "Consumer Thread"
  void processLoop();
  
  std::thread workerThread_;
  std::mutex mutex_;
  std::condition_variable_cv_;
  std::atomic<bool> isRunning_;

  // Shared Memory Space (Retains only latest frame)
  std::vector<uint8_t> latestFrame_;
  int frameWidth_;
  int frameHeight_;
  bool hasNewFrame_;

  AuthResult currentResult_;
};