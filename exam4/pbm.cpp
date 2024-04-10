#include "pbm.h"


bool BinaryImage::ReadFromPBM(const std::string& filename) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;

	std::string magic_number;
	uint8_t character;

	//magic number
	is >> magic_number;
	if (magic_number != "P4") return false;
	character = is.get();
	if (character != '\n') return false;

	//commento
	character = is.peek();
	if (character == '#') {
		while (1) {
			character = is.get();
			if (character == '\n') break;
		}
	}

	//width
	is >> W;
	character = is.get();
	if (character != ' ') return false;

	//height
	is >> H;
	character = is.get();
	if (character != '\n') return false;

	int w;
	if (W % 8 == 0) {
		w = W / 8;
	}
	else {
		w = W / 8 + 1;
	}

	ImageData.resize(H * w);
	for (int r = 0; r < H; ++r) {
		for (int c = 0; c < w; ++c) {
			ImageData[r * w + c] = is.get();
		}
	}

	return true;
}
/*
Image BinaryImageToImage(const BinaryImage& bimg) {
	Image img;
	img.H = bimg.H;
	img.W = bimg.W;

	uint8_t pixel = 0;
	int cols = bimg.ImageData.size() / bimg.H;
	int count = 0;

	for (int r = 0; r < img.H; ++r) {
		for (int c = 0; c < cols; ++c) {
			uint8_t pixels8 = bimg.ImageData[r * cols + c];
			for (int i = 0; i < 8; ++i) {
				if (count == img.W * img.H){
					return img;
				}
				pixel = (pixels8 >> (7-i)) & 0b00000001;
				count++;
				if (pixel == 1) {
					img.ImageData.push_back(0);
				}
				else {
					img.ImageData.push_back(255);
				}
			}
		}
	}
	return img;
}
*/
Image BinaryImageToImage(const BinaryImage& bimg) {
	Image img;
	img.H = bimg.H;
	img.W = bimg.W;
	img.ImageData.resize(img.H * img.W);

	uint8_t pixel = 0;
	int cols = bimg.ImageData.size() / bimg.H;

	for (int r = 0; r < img.H; ++r) {
		for (int c = 0; c < cols; ++c) {
			uint8_t pixels8 = bimg.ImageData[r * cols + c];
			for (int i = 0; i < 8; ++i) {
				if (r * img.W + c * 8 + i == img.W * img.H) {
					return img;
				}
				pixel = (pixels8 >> (7-i)) & 1;
				if (pixel == 1) {
					img.ImageData[r * img.W + c * 8 + i] = 0;
				}
				else {
					img.ImageData[r * img.W + c * 8 + i] = 255;
				}
			}
		}
	}
	return img;
}
/*
int main() {
	BinaryImage img;
	Image img2;
	if (img.ReadFromPBM("im1.pbm")) {
		img2 = BinaryImageToImage(img);
	}
	return 0;
}*/