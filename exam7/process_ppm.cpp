#include "ppm.h"
#include "mat.h"
#include <string>
#include <fstream>
#include "process_ppm.h"
/*
bool LoadPPM(const std::string& filename, mat<vec3b>& img) {
	std::string magic_number;
	int H, W, maxval;
	std::ifstream is(filename, std::ios_base::binary);
	
	is >> magic_number;
	if (magic_number != "P6") return false;
	if (is.get() != '\n') return false;

	if (is.peek() == '#') {
		while (is.get() != '\n') {}
	}
	is >> W;
	if (is.get() != ' ') return false;

	is >> H;
	if (is.get() != '\n') return false;

	is >> maxval;
	if (maxval != 255) return false;
	if (is.get() != '\n') return false;

	img.resize(H, W);
	vec3b pixel;

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			pixel[0] = is.get();
			pixel[1] = is.get();
			pixel[2] = is.get();
			img(r, c) = pixel;
		}
	}
	return true;
}


void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b) {
	img_r.resize(img.rows(), img.cols());
	img_g.resize(img.rows(), img.cols());
	img_b.resize(img.rows(), img.cols());
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			img_r(r, c) = img(r, c)[0];
			img_g(r, c) = img(r, c)[1];
			img_b(r, c) = img(r, c)[2];
		}
	}
}
/*
int main() {
	mat<vec3b> img;
	mat<uint8_t> img_r;
	mat<uint8_t> img_g;
	mat<uint8_t> img_b;

	LoadPPM("test.ppm", img);
	SplitRGB(img, img_r, img_g, img_b);
	return 0;
}*/