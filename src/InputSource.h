#pragma once

#include <array>
#include <string>

#include <libraw/libraw.h>

#include <Halide.h>
#include "finish.h"

class RawImage {
public:
    RawImage(unsigned char* buffer, size_t bsize, int width, int height, CfaPattern pattern);

    ~RawImage();

    int GetWidth();

    int GetHeight();

    int GetScalarBlackLevel() const;

    std::array<float, 4> GetBlackLevel() const;

    int GetWhiteLevel() const { return RawProcessor->imgdata.color.maximum; }

    WhiteBalance GetWhiteBalance() const;

    std::string GetCfaPatternString() const;

    CfaPattern GetCfaPattern() const;

    Halide::Runtime::Buffer<float> GetColorCorrectionMatrix() const;

    void CopyToBuffer(Halide::Runtime::Buffer<uint16_t>& buffer) const;

    // Writes current RawImage as DNG. If buffer was provided, then use it instead of internal buffer.
    void WriteDng(const std::string& path, const Halide::Runtime::Buffer<uint16_t>& buffer = {}) const;

    std::shared_ptr<LibRaw> GetRawProcessor() const { return RawProcessor; }
    std::shared_ptr<LibRaw> RawProcessor;
private:
    CfaPattern Pattern;
};
