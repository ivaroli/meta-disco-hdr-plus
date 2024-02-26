#include "InputSource.h"

#include <algorithm>
#include <unordered_map>

#include "LibRaw2DngConverter.h"

RawImage::RawImage(unsigned char* buffer, size_t bsize, int width, int height, CfaPattern pattern)
    : Pattern(pattern)
{
    // set the corrisponding libraw pattern
    LibRaw_openbayer_patterns librawPat = LIBRAW_OPENBAYER_BGGR;
    switch (pattern)
    {
    case CfaPattern::CFA_GBRG:
        librawPat = LIBRAW_OPENBAYER_GBRG;
        break;
    case CfaPattern::CFA_GRBG:
        librawPat = LIBRAW_OPENBAYER_GRBG;
        break;
    case CfaPattern::CFA_RGGB:
        librawPat = LIBRAW_OPENBAYER_RGGB;
        break;
    default:
        break;
    }

    this->RawProcessor = std::make_shared<LibRaw>();
    int ret = RawProcessor->open_bayer(buffer, bsize, width, height,0, 0, 0, 0, 0, librawPat, 0, 0, 1400);

    if (ret != LIBRAW_SUCCESS){
        std::cerr << "Cannot do open_bayer, error: " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("open_bayer error");
    }
    if ((ret = RawProcessor->unpack()) != LIBRAW_SUCCESS){
        std::cerr << "Cannot do unpack, error: " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("unpack error");
    }
    
    if ((ret = RawProcessor->dcraw_process()) != LIBRAW_SUCCESS){
        std::cerr << "Cannot do dcraw_process, error: " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("dcraw_process error");
    }
}

RawImage::~RawImage(){
}

int RawImage::GetWidth() { 
    std::cout << std::endl;

    return this->RawProcessor->imgdata.rawdata.sizes.width; 
}

int RawImage::GetHeight() { 
    return this->RawProcessor->imgdata.rawdata.sizes.height; 
}

WhiteBalance RawImage::GetWhiteBalance() const {
    const auto coeffs = RawProcessor->imgdata.rawdata.color.cam_mul;
    // Scale multipliers to green channel
    const float r = 2.5777f;//coeffs[0] / coeffs[1];
    const float g0 = 1.f; // same as coeffs[1] / coeffs[1];
    const float g1 = 1.f;
    const float b = 1.09253f;//coeffs[2] / coeffs[1];
    return WhiteBalance{r, g0, g1, b};
}

void RawImage::CopyToBuffer(Halide::Runtime::Buffer<uint16_t> &buffer) const {
    const auto image_data = (uint16_t*)RawProcessor->imgdata.rawdata.raw_image;
    const auto raw_width = RawProcessor->imgdata.rawdata.sizes.raw_width;
    const auto raw_height = RawProcessor->imgdata.rawdata.sizes.raw_height;
    const auto top = RawProcessor->imgdata.rawdata.sizes.top_margin;
    const auto left = RawProcessor->imgdata.rawdata.sizes.left_margin;
    Halide::Runtime::Buffer<uint16_t> raw_buffer(image_data, raw_width, raw_height);
    buffer.copy_from(raw_buffer.translated({-left, -top}));
}

void RawImage::WriteDng(const std::string &output_path, const Halide::Runtime::Buffer<uint16_t> &buffer) const
{
    LibRaw2DngConverter converter(*this);
    converter.SetBuffer(buffer);
    converter.Write(output_path);
}

std::string RawImage::GetCfaPatternString() const {
    if (this->Pattern == CfaPattern::CFA_RGGB) {
        return std::string{0, 1, 1, 2};
    } else if (this->Pattern == CfaPattern::CFA_GRBG) {
        return std::string{1, 0, 2, 1};
    } else if (this->Pattern == CfaPattern::CFA_BGGR) {
        return std::string{2, 1, 1, 0};
    } else if (this->Pattern == CfaPattern::CFA_GBRG) {
        return std::string{1, 2, 0, 1};
    }
}

std::array<float, 4> RawImage::GetBlackLevel() const {
    // See https://www.libraw.org/node/2471
    const auto raw_color = RawProcessor->imgdata.color;
    const auto base_black_level = static_cast<float>(raw_color.black);

    std::array<float, 4> black_level = {
        base_black_level + static_cast<float>(raw_color.cblack[0]),
        base_black_level + static_cast<float>(raw_color.cblack[1]),
        base_black_level + static_cast<float>(raw_color.cblack[2]),
        base_black_level + static_cast<float>(raw_color.cblack[3])
    };

    if (raw_color.cblack[4] == 2 && raw_color.cblack[5] == 2) {
        for (int x = 0; x < raw_color.cblack[4]; ++x) {
            for (int y = 0; y < raw_color.cblack[5]; ++y) {
                const auto index = y * 2 + x;
                black_level[index] = raw_color.cblack[6 + index];
            }
        }
    }

    return black_level;
}

int RawImage::GetScalarBlackLevel() const {
    const auto black_level = GetBlackLevel();
    return static_cast<int>(*std::min_element(black_level.begin(), black_level.end()));
}

CfaPattern RawImage::GetCfaPattern() const {
    return this->Pattern;
}

Halide::Runtime::Buffer<float> RawImage::GetColorCorrectionMatrix() const {
    const auto raw_color = RawProcessor->imgdata.color;
    Halide::Runtime::Buffer<float> ccm(3, 3);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            ccm(i, j) = raw_color.rgb_cam[j][i];
        }
    }
    return ccm;
}


