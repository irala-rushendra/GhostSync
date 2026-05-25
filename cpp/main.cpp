#include "engine.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void simulateCameraProducer(GhostSyncEngine& engine, int totalFrames) {
    std::cout << "[CAMERA] Hardware initialized. Starting 60 FPS stream..." << std::endl;
    
    for (int i = 1; i <= totalFrames; ++i) {
        // Create a dummy byte array representing a 1080p frame
        std::vector<uint8_t> dummyFrame(1920 * 1080 * 3, i % 255); 
        
        // Push to engine
        engine.pushFrame(dummyFrame, 1920, 1080);
        
        // Simulate ~60 FPS (1 frame every 16 milliseconds)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    std::cout << "[CAMERA] Stream finished." << std::endl;
}

void simulateReactUiPolling(GhostSyncEngine& engine, int durationSeconds) {
    std::cout << "[REACT UI] Polling engine for results..." << std::endl;
    
    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(durationSeconds)) {
        
        // Synchronous poll, exactly how JSI will call it
        AuthResult result = engine.getLatestResult();
        
        if (result.isLive && result.confidenceScore > 0.0f) {
            std::cout << "[REACT UI] -> Authenticated: " << result.userId 
                      << " | Score: " << result.confidenceScore 
                      << " | Engine Latency: " << result.processingTimeMs << "ms" << std::endl;
        }

        // React Native typically renders at 60 FPS (~16ms per loop)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main() {
    std::cout << "--- GHOSTSYNC BARE-METAL TEST ---" << std::endl;
    
    // 1. Initialize the Engine (This immediately spins up the Consumer thread)
    GhostSyncEngine engine;
    
    // 2. Spin up the Camera Producer thread (Simulating the mobile hardware)
    std::thread cameraThread(simulateCameraProducer, std::ref(engine), 100); // Send 100 frames
    
    // 3. Spin up the React UI polling thread (Simulating the JavaScript layer)
    std::thread uiThread(simulateReactUiPolling, std::ref(engine), 2); // Poll for 2 seconds
    
    // 4. Wait for simulations to finish
    cameraThread.join();
    uiThread.join();
    
    std::cout << "--- TEST COMPLETE. ENGINE SHUTTING DOWN SAFELY ---" << std::endl;
    return 0;
}