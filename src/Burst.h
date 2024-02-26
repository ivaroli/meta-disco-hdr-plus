#pragma once

#include "InputSource.h"

#include <Halide.h>

#include <string>
#include <vector>

class Burst {
public:
    Burst(unsigned char* buffer, size_t fSize, size_t numImages, int width, int height, CfaPattern pattern)
        : Raws(LoadRaws(buffer, fSize, numImages, width, height, pattern))
    {}

    ~Burst() = default;

    int GetWidth();

    int GetHeight();

    int GetBlackLevel() const { return Raws.empty() ? -1 : Raws[0].GetScalarBlackLevel(); }

    int GetWhiteLevel() const { return Raws.empty() ? -1 : Raws[0].GetWhiteLevel(); }

    WhiteBalance GetWhiteBalance() const { return Raws.empty() ? WhiteBalance{-1, -1, -1, -1} : Raws[0].GetWhiteBalance(); }

    CfaPattern GetCfaPattern() const { return Raws.empty() ? CfaPattern::CFA_UNKNOWN : Raws[0].GetCfaPattern(); }
    
    Halide::Runtime::Buffer<float> GetColorCorrectionMatrix() const { return Raws.empty() ? Halide::Runtime::Buffer<float>() : Raws[0].GetColorCorrectionMatrix(); }

    Halide::Runtime::Buffer<uint16_t> ToBuffer();

    void CopyToBuffer(Halide::Runtime::Buffer<uint16_t>& buffer);

    const RawImage& GetRaw(const size_t i) const;

public:
    std::vector<RawImage> Raws;

private:
    static std::vector<RawImage> LoadRaws(unsigned char* buffer, size_t fSize, size_t numImages, int width, int height, CfaPattern pattern);
};
