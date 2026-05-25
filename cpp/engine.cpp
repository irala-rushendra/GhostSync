/*
author : irala rushendra
email  : iralarushendra@gmail.com
*/

#include "engine.h"
#include <iostream>
#include <chrono>

using namespace std::chrono;

GhostSyncEngine::GhostSyncEngine() : isRunning_(true), hasNewFrame_(false), frameWidth_(0), frameHeight_(0) 
{
    // Default state initialization
    currentResult_ = {false, 0.0f, "", 0};
    // workerThread_ starts the moment Engine is created
    workerThread_ = std::thread(&GhostSyncEngine::processLoop, this);
}

GhostSyncEngine::~GhostSyncEngine() {
    isRunning_ = false;
    cv_.notify_one();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void GhostSyncEngine::pushFrame(const std::vector<uint8_t>& frameData, int width, int height) {
    // Scope-based lock: automatically unlocks when the function finishes
    std::lock_guard<std::mutex> lock(mutex_);
    
    latestFrame_ = frameData;
    frameWidth_ = width;
    frameHeight_ = height;
    hasNewFrame_ = true;
    
    // Signal to wake up the Consumer thread
    cv_.notify_one();
}

AuthResult GhostSyncEngine::getLatestResult() {
    std::lock_guard<std::mutex> lock(mutex_);
    // Return a safe copy of the most recent authentication data to the JS thread
    return currentResult_;
}

void GhostSyncEngine::processLoop() {
    while (isRunning_) {
        std::vector<uint8_t> localFrame;
        int localWidth = 0;
        int localHeight = 0;
        {
            // Locks the memory just long enough to fetch the frame
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Go to sleep until "hasNewFrame_" is true or if being shutting down
            cv_.wait(lock, [this] { return hasNewFrame_ || !isRunning_; });
            
            if (!isRunning_) break;

            localFrame = latestFrame_;
            localWidth = frameWidth_;
            localHeight = frameHeight_;
            
            // prevents processing same frame twice or more
            hasNewFrame_ = false;
        } 

        // NCNN Inference Pipeline Section
        auto startTime = std::chrono::high_resolution_clock::now();
        
        bool livenessPassed = false;
        float faceMatchScore = 0.0f;
        std::string authUser = "";

        // TODO: In model_runner.cpp, we will implement the actual NCNN execution:
        // 1. Convert localFrame (YUV/RGB) to ncnn::Mat
        // 2. Run BlazeFace -> Check eye/head landmarks (Liveness)
        // 3. If Live -> Crop face -> Run MobileFaceNet
        // 4. Calculate Cosine Similarity against local database

        // --- Mocking the NCNN processing time for testing ---
        std::this_thread::sleep_for(std::chrono::milliseconds(210)); 
        livenessPassed = true; 
        faceMatchScore = 0.97f; 
        authUser = "USR_8832";
        // ----------------------------------------------------

        auto endTime = high_resolution_clock::now();
        auto durationMs = duration_cast<milliseconds>(endTime - startTime).count();

        // Lock briefly to update the result state for the JS thread inside a scope
        {
            std::lock_guard<std::mutex> lock(mutex_);
            currentResult_.isLive = livenessPassed;
            currentResult_.confidenceScore = faceMatchScore;
            currentResult_.userId = authUser;
            currentResult_.processingTimeMs = durationMs;
        }
    }
}