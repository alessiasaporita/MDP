#ifndef PBM_H
#define PBM_H

#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <exception>

struct BinaryImage {
    int W;
    int H;
    std::vector<uint8_t> ImageData;

    bool ReadFromPBM(const std::string& filename);

};

struct Image {
    int W;
    int H;
    std::vector<uint8_t> ImageData;
};

Image BinaryImageToImage(const BinaryImage& bimg);
//bool BinaryImage::ReadFromPBM(const std::string& filename);

#endif // !PBM_H

