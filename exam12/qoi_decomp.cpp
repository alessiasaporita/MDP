#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cstring>

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows*cols) {}

    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

struct qoi_header{
 char magic[4]; // magic bytes "qoif"
 uint32_t width; // image width in pixels (BE)
 uint32_t height; // image height in pixels (BE)
 uint8_t channels; // 3 = RGB, 4 = RGBA
 uint8_t colorspace; // 0 = sRGB with linear alpha
                     // 1 = all channels linear
};

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&val), size);
}

using rgba = std::array<uint8_t, 4>;

void decompress(std::istream& is, mat<rgba>& img, qoi_header q) {
    rgba dict[64] = { 0 };
    int count = 0;
    rgba pixel;
    uint8_t tag;
    int index_pos;
    rgba starter = { 0, 0, 0, 255 };
    int counter = 0;

    while (1) {
        if (counter == q.width * q.height) break;
        tag = is.get();
        rgba prec = (counter == 0) ? starter : img.data_[counter - 1];

        if (tag == 0b11111110) { // full r,g,b values
            count = 0;
            pixel[0] = is.get();
            pixel[1] = is.get();
            pixel[2] = is.get();
            pixel[3] = prec[3];
        }
        else if (tag == 0b11111111) { // r,g,b,a values
            count = 0;
            pixel[0] = is.get();
            pixel[1] = is.get();
            pixel[2] = is.get();
            pixel[3] = is.get();
        }
        else if (((tag >> 6) & 0b00000011) == 0b00) { //an index into an array of previously seen pixels
            uint8_t index = tag & 0b00111111;
            if (tag == 0) count++;
            if (count == 7) {
                tag = is.get();
                if (tag == 1) {
                    for (size_t i = 0; i < 6; ++i) { //rimuovo gli ultimi sei elementi di data
                        img.data_.pop_back();
                    }
                    break; //end marker
                }
                else {
                    index = tag & 0b00111111;
                    img.data_.push_back(dict[0]);
                }
            }
            pixel = dict[index];
        }
        else if (((tag >> 6) & 0b00000011) == 0b01) { //a difference to the previous pixel value in r,g,b
            count = 0;
            char dr = (tag >> 4) & 0b00000011;
            char dg = (tag >> 2) & 0b00000011;
            char db = tag & 0b00000011;
            //conversione
            pixel[0] = prec[0] + dr - 2;
            pixel[1] = prec[1] + dg - 2;
            pixel[2] = prec[2] + db - 2;
            pixel[3] = prec[3];
        }
        else if (((tag >> 6) & 0b00000011) == 0b10) { //a difference to the previous pixel value in r,g,b
            count = 0;
            char dg = tag & 0b00111111;
            uint8_t byte = is.get();
            char dr_dg = (byte >> 4) & 0b00001111;
            char db_dg = byte & 0b00001111;
            //conversione
            pixel[1] = prec[1] + dg - 32;
            pixel[0] = prec[0] + dr_dg + pixel[1] - prec[1] - 8;
            pixel[2] = prec[2] + db_dg + pixel[1] - prec[1] - 8;
            pixel[3] = prec[3];
        }
        else if (((tag >> 6) & 0b00000011) == 0b11) { //a run of the previous pixel
            count = 0;
            uint8_t run = tag & 0b00111111;
            run++;
            for (size_t i = 0; i < run; ++i) {
                img.data_[counter] = prec;
                counter++;
            }
            continue;
        }
        index_pos = (pixel[0] * 3 + pixel[1] * 5 + pixel[2] * 7 + pixel[3] * 11) % 64;
        dict[index_pos] = pixel;
        img.data_[counter] = pixel;
        counter++;
    }
    return;
}

bool readheader(std::istream& is, qoi_header& q) {
    is.read(reinterpret_cast<char*>(&q.magic[0]), 4);
    if (strncmp(q.magic, "qoif", 4) != 0) return false;
    bitreader br(is);
    q.width = br(32);
    q.height = br(32);
    q.channels = is.get();
    q.colorspace = is.get();
    return true;
}

int main(int argc, char* argv[])
{
    if (argc != 3) return 1;
    std::ifstream is(argv[1], std::ios_base::binary);
    if (!is) return 1;
    qoi_header q;
    if (!readheader(is, q)) return 1;
    mat<rgba> img(q.height, q.width);

    decompress(is, img, q);  
 
    std::ofstream os(argv[2], std::ios::binary); 
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}