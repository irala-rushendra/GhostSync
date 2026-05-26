#include <opencv2/opencv.hpp>
#include "engine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void uiPollingThread(GhostSyncEngine& engine) {
    while (true) {
        AuthResult result = engine.getLatestResult();
        if (result.isLive) {
            std::cout << "[UI] FACE DETECTED! | Score: " << result.confidenceScore 
                      << " | Latency: " << result.processingTimeMs << "ms\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::cout << "--- STARTING REAL VISION TEST ---" << std::endl;

    GhostSyncEngine engine;
    
    // Initialize the engine by forcing it to load the models from the current directory
    // We need to bypass the privacy of engine for this quick test, so we'll just mock the load
    // Assuming you call loadModels inside the engine constructor or init function.
    // For this test, ensure your engine calls `modelRunner_.loadModels(".")` when it boots up.

    cv::VideoCapture cap(0); // Try to open /dev/video0 (Webcam)
    
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] Cannot open webcam. Are you on WSL without usbipd?" << std::endl;
        std::cerr << "Fallback: Put a file named 'test_face.jpg' in this folder to test statically." << std::endl;
        
        cv::Mat staticImage = cv::imread("/home/rushendra/projects/GhostSync/cpp/test_face.jpg");
        if (staticImage.empty()) return -1;
        
        cv::cvtColor(staticImage, staticImage, cv::COLOR_BGR2RGBA);
        std::vector<uint8_t> frameData(staticImage.data, staticImage.data + (staticImage.total() * 4));
        engine.pushFrame(frameData, staticImage.cols, staticImage.rows);
        
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Give engine time to process
        return 0;
    }

    std::thread ui(uiPollingThread, std::ref(engine));

    cv::Mat frame, rgbaFrame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Convert OpenCV BGR format to RGBA byte array for our engine
        cv::cvtColor(frame, rgbaFrame, cv::COLOR_BGR2RGBA);
        std::vector<uint8_t> frameData(rgbaFrame.data, rgbaFrame.data + (rgbaFrame.total() * 4));

        engine.pushFrame(frameData, rgbaFrame.cols, rgbaFrame.rows);

        // Display the raw feed (Optional, might crash X11 forwarding on WSL)
        // cv::imshow("Webcam", frame); 
        // if (cv::waitKey(1) == 27) break; // ESC to quit
    }

    ui.join();
    return 0;
}