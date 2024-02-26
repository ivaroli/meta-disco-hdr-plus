#include "Burst.h"

Halide::Runtime::Buffer<uint16_t> Burst::ToBuffer() {
    if (Raws.empty()) {
        return Halide::Runtime::Buffer<uint16_t>();
    }

    Halide::Runtime::Buffer<uint16_t> result(GetWidth(), GetHeight(), Raws.size());
    for (int i = 0; i < Raws.size(); ++i) {
        auto resultSlice = result.sliced(2, i);
        Raws[i].CopyToBuffer(resultSlice);
    }
    return result;
}

void Burst::CopyToBuffer(Halide::Runtime::Buffer<uint16_t> &buffer) {
    buffer.copy_from(ToBuffer());
}

std::vector<RawImage> Burst::LoadRaws(unsigned char* buffer, size_t fSize, size_t numImages, int width, int height, CfaPattern pattern) {
    std::vector<RawImage> result;
    
    for(size_t i = 0; i < numImages; i++){
        result.emplace_back(&buffer[i*fSize], fSize, width, height, pattern);
    }
    
    return result;
}

const RawImage& Burst::GetRaw(const size_t i) const {
    return this->Raws[i];
}

int Burst::GetWidth() { return Raws.empty() ? -1 : Raws[0].GetWidth(); }

int Burst::GetHeight() { return Raws.empty() ? -1 : Raws[0].GetHeight(); }
