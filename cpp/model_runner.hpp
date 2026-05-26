/*
author : irala rushendra
email  : iralarushendra@gmail.com
*/
#pragma once

#include <vector>
#include <string>
#include <net.h>
#include <mat.h>

class GhostSyncModelRunner {
public:
    GhostSyncModelRunner();
    ~GhostSyncModelRunner();

    // Prevent copying and reassignment
    GhostSyncModelRunner(const GhostSyncModelRunner&) = delete;
    GhostSyncModelRunner& operator=(const GhostSyncModelRunner&) = delete;

    // Load the INT8 quantized models from the asset directory
    bool loadModels(const std::string& assetDir);

    // The main execution pipeline
    // Returns: pair<bool isLive, float confidenceScore>
    std::pair<bool, float> runInference(const std::vector<uint8_t>& frameData, int width, int height);

private:
    ncnn::Net blazefaceNet_;
    ncnn::Net mobilefacenetNet_;

    // Helper functions
    bool checkLiveness(const ncnn::Mat& blazefaceOutput);
    std::vector<float> extractFaceEmbedding(const ncnn::Mat& faceCrop);
    float calculateCosineSimilarity(const std::vector<float>& embed1, const std::vector<float>& embed2);
    
    // Hardcoded mock embedding of the "authorized user" for local testing
    std::vector<float> targetUserEmbedding_; 
};