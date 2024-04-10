#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <math.h>  
#include <limits>

using vec3b = std::array<uint8_t, 3>;
using vec3bf = std::array<float, 3>;


template <typename T>
struct mat {
	int rows_, cols_;
	std::vector<T> data_;

	mat(){}
	mat(int rows, int cols): rows_(rows), cols_(cols), data_(rows*cols){}

	void resize(int rows, int cols) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}

	T& operator()(int r, int c) { return data_[r * cols_ + c]; }
	const T& operator()(int r, int c) const{ return data_[r * cols_ + c]; }

	int rows() { return rows_; }
	int cols() { return cols_; }
};

bool save_PAM(mat<vec3b> img, std::ostream& os) {
	os << "P7\n";
	os << "WIDTH " << img.cols() << "\n";
	os << "HEIGHT " << img.rows() << "\n";
	os << "DEPTH 3\n";
	os << "MAXVAL 255\n";
	os << "TUPLTYPE RGB\n";
	os << "ENDHDR\n";

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			os.put(img(r, c)[0]);
			os.put(img(r, c)[1]);
			os.put(img(r, c)[2]);
		}
	}
	return true;
}

bool read_header(std::istream& is) {
	std::string magic_number, vname, text;
	is >> magic_number;
	if (magic_number != "#?RADIANCE") return false;
	if (is.get() != '\n') return false;

	while (1) {
		if (is.peek() == '#') { //commento
			while (is.get() != '\n') {}
		}
		if (is.peek() == '\n') {
			is.get();
			break; //fine header
		}
		std::getline(is, vname, '=');
		std::getline(is, text);
		if (vname == "FORMAT" && text != "32-bit_rle_rgbe") return false;
	}
	return true;
}

void check_min_max(float& min, float& max, float& val) {
	if (val < min) {
		min = val;
	}
	if (val > max) {
		max = val;
	}
}
bool decompress(std::istream& is, mat<vec3b>& img) {
	float min = std::numeric_limits<float>::max();
	float max = std::numeric_limits<float>::min();
	std::vector<uint8_t> scanline;
	int r = 0; 
	uint16_t M_BE = 0;
	int dim_scanline = img.cols() * 4;
	mat<vec3bf> img2(img.rows(), img.cols());

	//leggo la matrice HDR image
	while (r<img.rows()) {
		//header
		if (is.get() != 0x02) return false;
		if (is.get() != 0x02) return false;
		M_BE = is.get();
		M_BE = (M_BE << 8) | is.get();
		if (M_BE != img.cols()) return false;
		//leggo una scanline
		while (scanline.size() < dim_scanline) {
			uint8_t L = is.get();
			if (L <= 127) { //copy
				for (int i = 0; i < L; ++i) {
					uint8_t next = is.get();
					scanline.push_back(next);
				}
			}
			else { //run
				uint8_t next = is.get();
				for (int i = 0; i < L -  128; ++i) {
					scanline.push_back(next);
				}
			}
		}

		//inserisco nell'immagine
		for (int c = 0; c < img.cols(); ++c) {
			uint8_t E = scanline[c + 3 * img.cols()];
			float R = (scanline[c] + 0.5) / 256.0 * std::pow(2, E - 128);
			check_min_max(min, max, R);
			float G = (scanline[c + img.cols()] + 0.5) / 256.0 * std::pow(2, E - 128);
			check_min_max(min, max, G);
			float B = (scanline[c + 2 * img.cols()] + 0.5) / 256.0 * std::pow(2, E - 128);
			check_min_max(min, max, B);
			img2(r, c)[0] = R;
			img2(r, c)[1] = G;
			img2(r, c)[2] = B;
		}
		++r;
		scanline.resize(0);
	}

	//map linear luminance
	float base = 0.0;
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			base = (img2(r, c)[0] - min) / (max - min);
			img(r, c)[0] = 255 * std::pow(base, 0.45);

			base = (img2(r, c)[1] - min) / (max - min);
			img(r, c)[1] = 255 * std::pow(base, 0.45);

			base = (img2(r, c)[2] - min) / (max - min);
			img(r, c)[2] = 255 * std::pow(base, 0.45);
		}
	}
	return true;
}
int main(int argc, char* argv[]) {
	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return EXIT_FAILURE;

	if (!read_header(is)) {
		return EXIT_FAILURE;
	}

	mat<vec3b> img;
	int h, w;

	//resolution string
	std::string stringa;

	is >> stringa;
	if (stringa != "-Y") return EXIT_FAILURE;
	if (is.get() != ' ') return EXIT_FAILURE;
	is >> h;
	if (is.get() != ' ') return EXIT_FAILURE;
	is >> stringa;
	if (stringa != "+X") return EXIT_FAILURE;
	if (is.get() != ' ') return EXIT_FAILURE;
	is >> w;
	if (is.get() != '\n') return EXIT_FAILURE;

	img.resize(h, w);
	decompress(is, img);

	std::ofstream os(argv[2], std::ios_base::binary);
	if (!os) return EXIT_FAILURE;
	save_PAM(img, os);

	return EXIT_SUCCESS;
}