#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <cstring>

using rgba = std::array<uint8_t, 4>;

template<typename T>
struct mat {
    size_t rows_, cols_;
    std::vector<T> data_;

    mat(size_t rows, size_t cols) : rows_(rows), cols_(cols), data_(rows*cols) {}
    
    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }
    const T& operator()(size_t r, size_t c) const { return data_[r * cols_ + c]; }
    T& operator()(size_t r, size_t c) { return data_[r * cols_ + c]; }

    size_t size() const { return rows_ * cols_; }
    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }

    const char* rawdata() const {
        return reinterpret_cast<const char*>(data_.data());
    }
    size_t rawsize() const { return size() * sizeof(T); }
};

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&val), size);
}

struct qoi_header {
    char magic[4]; // magic bytes "qoif"
    uint32_t width; // image width in pixels (BE)
    uint32_t height; // image height in pixels (BE)
    uint8_t channels; // 3 = RGB, 4 = RGBA
    uint8_t colorspace; // 0 = sRGB with linear alpha, 1 = all channels linear
};

bool read_header(std::istream& is, qoi_header& h) {
    raw_read(is, h.magic);
    if (std::strncmp(h.magic, "qoif", 4) != 0) return false;
    h.width = is.get();
    h.width = (h.width << 8) | is.get();
    h.width = (h.width << 8) | is.get();
    h.width = (h.width << 8) | is.get();
    h.height = is.get();
    h.height = (h.height << 8) | is.get();
    h.height = (h.height << 8) | is.get();
    h.height = (h.height << 8) | is.get();
    h.channels = is.get();
    if (h.channels != 3 && h.channels != 4) return false;
    h.colorspace = is.get();
    if (h.colorspace != 0 && h.colorspace != 1) return false;
    return true;
}

int hash_function(rgba pixel) {
    int r = pixel[0];
    int g = pixel[1];
    int b = pixel[2];
    int a = pixel[3];
    return ((r * 3 + g * 5 + b * 7 + a * 11) % 64); 
}

bool decode(std::istream& is, mat<rgba>& img) {
    // decodificare il file QOI in input e inserire i dati nell'immagine di output
    rgba prec{ 0, 0, 0, 255 };
    rgba dict[64]{ 0 };
    uint8_t byte;
    size_t count = 0;
    size_t index_pos;
    int eos = 0;
    rgba pixel;
    while (count < img.size()) {
        byte = is.get();
        if (byte == 0b11111110) { // full r,g,b
            eos = 0;
            pixel[0] = is.get();
            pixel[1] = is.get();
            pixel[2] = is.get();
            pixel[3] = prec[3];
        }
        else if (byte == 0b11111111) { // rgba
            eos = 0;
            pixel[0] = is.get();
            pixel[1] = is.get();
            pixel[2] = is.get();
            pixel[3] = is.get();
        }
        else if (((byte >> 6) & 0b00000011) == 0) { //index into an array of previously seen pixels
            size_t index = byte & 0b00111111;
            pixel = dict[index];
            if (index == 0) {
                eos++;
            }
            
            if (eos == 7 && index == 1) {
                for (size_t i = 0; i < 6; ++i) { //rimuovo gli ultimi sei elementi di data
                    img.data_.pop_back();
                }
                break; //byte stream's end marker
            }
        }
        else if (((byte >> 6) & 0b00000011) == 1) { // difference to the previous pixel value in r,g,b
            eos = 0;
            char dr, dg, db;
            dr = ((byte >> 4) & 0b00000011) - 2;
            dg = ((byte >> 2) & 0b00000011) - 2;
            db = (byte & 0b00000011) - 2;
            pixel[0] = prec[0] + dr;
            pixel[1] = prec[1] + dg;
            pixel[2] = prec[2] + db;
            pixel[3] = prec[3];
        }
        else if (((byte >> 6) & 0b00000011) == 2) {// difference to the previous pixel value in r,g,b
            eos = 0;
            char dg = (byte & 0b00111111) - 32;
            uint8_t next = is.get();
            char dr_dg = ((next >> 4) & 0b00001111) - 8;
            char db_dg = (next & 0b00001111) - 8;
            pixel[1] = prec[1] + dg; //cur_px.g 
            //dr_dg + (dg) = img[count][0] - prec[0]
            pixel[0] = prec[0] + dr_dg + dg;
            pixel[2] = prec[2] + db_dg + dg;
            pixel[3] = prec[3];
        }
        else if (((byte >> 6) & 0b00000011) == 3) { // run
            eos = 0;
            uint8_t lrun = (byte & 0b00111111) + 1;
            for (uint8_t i = 0; i < lrun; ++i) {
                img[count] = prec;
                count++;
                if (count == img.size()) break;
            }
            continue;
        }
        else {
            return EXIT_FAILURE;
        }
        img[count] = pixel;
        prec = pixel;
        index_pos = hash_function(pixel);
        dict[index_pos] = pixel;
        count++;
    }
    return EXIT_SUCCESS;
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    std::ifstream is(argv[1], std::ios_base::binary);
    if (!is) {
        return EXIT_FAILURE;
    }

    qoi_header h;
    if (!read_header(is, h)) {
        return EXIT_FAILURE;
   }

    mat<rgba> img(h.height, h.width); 

    if (decode(is, img) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    std::ofstream os(argv[2], std::ios::binary); 
    os << "P7\nWIDTH " << img.cols() << "\nHEIGHT " << img.rows() <<
        "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    os.write(img.rawdata(), img.rawsize());

    return EXIT_SUCCESS;
}