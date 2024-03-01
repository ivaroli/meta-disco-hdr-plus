#include <stdio.h>
#include <iostream>
#include <fstream>

#include <Halide.h>

#include <src/Burst.h>

#include <align_and_merge.h>

Halide::Runtime::Buffer<uint16_t> align_and_merge(Halide::Runtime::Buffer<uint16_t> burst) {
    if (burst.channels() < 2) {
        return {};
    }
    Halide::Runtime::Buffer<uint16_t> merged_buffer(burst.width(), burst.height());
    align_and_merge(burst, merged_buffer);
    return merged_buffer;
}

unsigned char* readBinaryFile(const std::string& filename, size_t fileSize) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return nullptr;
    }

    unsigned char* buffer = new unsigned char[fileSize];
    file.read(reinterpret_cast<char*>(buffer), fileSize);
    file.close();

    return buffer;
}

size_t getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return 0;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.close();

    return fileSize;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " dir_path out_img raw_img1 raw_img2 [...]" << std::endl;
        return 1;
    }

    int i = 1;
    if (argc - i < 3) {
        std::cerr << "Usage: " << argv[0] << " dir_path out_img raw_img1 raw_img2 [...]" << std::endl;
        return 1;
    }

    const std::string dir_path = argv[i++];
    const std::string out_name = argv[i++];

    std::vector<std::string> in_names;
    while (i < argc) {
        in_names.emplace_back(argv[i++]);
    }

    // Hardcoded test values
    CfaPattern pattern = CfaPattern::CFA_BGGR;
    size_t fileSize = getFileSize(dir_path + in_names.at(0));
    unsigned char* buffer = new unsigned char[in_names.size()*fileSize];
    size_t WIDTH=2592, HEIGHT=1944;

    for(size_t i = 0; i < in_names.size(); i++){
        std::string f = in_names.at(i);
        unsigned char* fBuffer = readBinaryFile(dir_path + f, fileSize);
        memcpy(&buffer[i*fileSize], fBuffer, fileSize);
        delete[] fBuffer;
    }

    Burst burst(buffer, fileSize, in_names.size(), WIDTH, HEIGHT, pattern);

    const auto merged = align_and_merge(burst.ToBuffer());
    std::cerr << "merged size: " << merged.width() << " " << merged.height() << std::endl;

    const RawImage& raw = burst.GetRaw(0);
    const std::string merged_filename = dir_path + "/" + out_name;
    raw.WriteDng(merged_filename, merged);

    delete[] buffer;
    return EXIT_SUCCESS;
}
