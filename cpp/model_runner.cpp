#include "model_runner.hpp"
#include <iostream>
#include <thread>
#include <chrono>

GhostSyncModelRunner::GhostSyncModelRunner() {
    targetUserEmbedding_.assign(128, 0.5f); 
}

GhostSyncModelRunner::~GhostSyncModelRunner() {
    blazefaceNet_.clear();
    mobilefacenetNet_.clear();
}

bool GhostSyncModelRunner::loadModels(const std::string& assetDir) {
    ncnn::Option opt;
    opt.lightmode = true;
    opt.num_threads = 2;
    blazefaceNet_.opt = opt;

    // Load SqueezeNet to test the inference engine
    if (blazefaceNet_.load_param((assetDir + "/squeezenet_v1.1.param").c_str()) != 0 ||
        blazefaceNet_.load_model((assetDir + "/squeezenet_v1.1.bin").c_str()) != 0) {
        std::cerr << "[ENGINE] Failed to load models." << std::endl;
        return false;
    }
    std::cout << "[ENGINE] Vision Model loaded successfully." << std::endl;
    return true;
}

std::pair<bool, float> GhostSyncModelRunner::runInference(const std::vector<uint8_t>& frameData, int width, int height) {
    ncnn::Mat in = ncnn::Mat::from_pixels(frameData.data(), ncnn::Mat::PIXEL_RGBA2RGB, width, height);

    // Resize down to 227x227 (SqueezeNet's required input size)
    ncnn::Mat in_resized;
    ncnn::resize_bilinear(in, in_resized, 227, 227);
    
    const float mean_vals[3] = {104.f, 117.f, 123.f};
    in_resized.substract_mean_normalize(mean_vals, 0);

    // --- EXECUTE THE NEURAL NETWORK ---
    ncnn::Extractor ex = blazefaceNet_.create_extractor();
    ex.input("data", in_resized);
    
    ncnn::Mat out;
    ex.extract("prob", out); // SqueezeNet's output layer is named 'prob'
    
    // If 'out' is not empty, the neural network successfully processed the image!
    if (!out.empty()) {
        return {true, 0.95f}; 
    }

    return {false, 0.0f}; 
}
// ... keep checkLiveness, extractFaceEmbedding, and calculateCosineSimilarity as dummy returns for now ...