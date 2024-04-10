#include <stdlib.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include "ppm.h"

template<typename T>
int saturate(T& val, int index) { //0=Y, 1=Cb/Cr, 2=RGB
	if (index == 0) { 
		if (val < 16) val = 16;
		if (val > 235) val = 235;
	}
	else if(index == 1) {
		if (val < 16) val = 16;
		if (val > 240) val = 240;
	}
	else {
		if (val < 0) val = 0;
		if (val > 255) val = 255;
	}
	return val;
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
	
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;

	std::string stream_header;
	is >> stream_header;
	if (stream_header != "YUV4MPEG2") return false;

	int w = -1, h = -1;

	while (1) {
		char character = is.get();
		if (character == '\n') break;
		if (character != ' ') return false;
		character = is.get();
		switch(character){
			case 'W': {
				is >> w;
				continue;
			}
			case 'H': {
				is >> h;
				continue;
			}
			case 'C': {
				std::string chroma;
				is >> chroma;
				if (chroma != "420jpeg") return false;
				continue;
			}
			case 'I': {
				char interlacing;
				is >> interlacing;
				if (interlacing != 'p') return false;
				continue;
			}
			case 'F': {
				int num, den;
				char op;
				is >> num >> op >> den;
				continue;
			}
			case 'A': {
				int num, den;
				char op;
				is >> num >> op >> den;
				continue;
			}
			case 'X': {
				std::string field;
				is >> field;
				continue;
			}
			default: return false;
		}
		return false;
	}

	if (h < 0 || w < 0) return false;

	std::string frame_header;
	mat<uint8_t> Y(h, w);
	mat<uint8_t> Cb(h / 2, w / 2);
	mat<uint8_t> Cr(h / 2, w / 2);
	mat<vec3b> RGB(h, w);

	while (is >> frame_header) {
		if (frame_header != "FRAME") return false;

		while (1) {
			char character = is.get();
			if (character == '\n') break;
			if (character != ' ') return false;
			character = is.get();
			switch (character) {
			case 'I': {
				char interlacing;
				is >> interlacing;
				if (interlacing != 'p') return false;
				continue;
			}
			case 'X': {
				std::string field;
				is >> field;
				continue;
			}
			default: return false;
			}
			return false;
		}

		is.read(Y.rawdata(), Y.rawsize());
		is.read(Cb.rawdata(), Cb.rawsize());
		is.read(Cr.rawdata(), Cr.rawsize());

		//RGB
		for (int r = 0; r < RGB.rows(); ++r) {
			for (int c = 0; c < RGB.cols(); ++c) {
				vec3b rgb;
				int R, G, B;
				int y = saturate(Y(r, c), 0);
				int cb = saturate(Cb(r / 2, c / 2), 1);
				int cr = saturate(Cr(r / 2, c / 2), 1);

				R = 1.164 * (y - 16) + 1.596 * (cr - 128);
				G = 1.164 * (y - 16) - 0.392 * (cb - 128) - 0.813 * (cr - 128);
				B = 1.164 * (y - 16) + 2.017 * (cb - 128);
				
				rgb[0] = saturate(R, 2);
				rgb[1] = saturate(G, 2);
				rgb[2] = saturate(B, 2);

				RGB(r, c) = rgb;
			}
		}
		frames.push_back(RGB);
	}
	return true;
}



/*
int main() {
	std::vector<mat<vec3b>> frames;

	if (y4m_extract_color("foreman_cif.y4m", frames)) {
		for (size_t i = 0; i < frames.size(); ++i) {
			std::stringstream ss;
			ss << "frame" << i << ".ppm";
			save_ppm(ss.str(), frames[i]);
		}
	}
	return 0;
}
*/