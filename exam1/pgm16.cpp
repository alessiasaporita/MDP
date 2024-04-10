#include "pgm16.h"
#include <fstream>

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	std::string magic_number;
	std::string comment;
	uint8_t character;
	int H, W;
	uint32_t max;
	uint16_t elem;

	//magic_number
	is >> magic_number;
	if (magic_number != "P5") return false;
	is.read(reinterpret_cast<char *>(&character), 1);
	if (character != '\n') return false;

	//comment
	character = is.peek();
	if (character == '#') {
		character = is.get();
		while (is >> comment) {
			is.read(reinterpret_cast<char*>(&character), 1);
			if (character == '\n') break;
		}
	}

	//width 
	is >> W;
	is.read(reinterpret_cast<char*>(&character), 1);
	if (character != ' ') return false;
	
	//height
	is >> H;
	is.read(reinterpret_cast<char*>(&character), 1);
	if (character != '\n') return false;

	//max value
	is >> max;
	if (max > 65535) return false;
	is.read(reinterpret_cast<char*>(&character), 1);
	if (character != '\n') return false;
	maxvalue = max;

	img.resize(H, W);
	if (max < 256) {
		for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				img(r, c) = is.get();
			}
		}
		//if (!is.read(img.rawdata(), H * W)) return false;
	}
	else {
		for (int r = 0; r < img.rows(); ++r) {
			for (int c = 0; c < img.cols(); ++c) {
				elem = is.get();
				elem = elem << 8;
				elem += is.get();
				img(r, c) = elem;
			}
		}
	}
	return true;
}

/*
int main() {
	mat<uint16_t> img;
	uint16_t max_value;
	bool ris = load("frog_bin.pgm", img, max_value);
	return 0;
}*/