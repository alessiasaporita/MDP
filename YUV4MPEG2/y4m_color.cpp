#include "types.h"
#include <string>
#include <vector>
#include "mat.h"
#include <fstream>
#include "ppm.h"

bool read_header(std::istream& is, int& h, int& w) {
	std::string magic_number;
	is >> magic_number;
	if (magic_number != "YUV4MPEG2") return false;
	while (1) {
		char c = is.get();
		if (c == '\n') return true; //fine header
		if (c != ' ') return false;
		switch (is.get())
		{
		break; case 'W': {
			is >> w;
		}
		break; case 'H': {
			is >> h;
		}
		break; case 'C': {
			std::string chroma;
			is >> chroma;
			if (chroma != "420jpeg") return false;
		}
		break; case 'I': {
			char I = is.get();
			if (I != 'p') return false;
		}
		break; case 'F': {
			char op; int den, num;
			is >> num >> op >> den;
			if (op != ':') return false;
		}
		break; case 'A': {
			char op; int den, num;
			is >> num >> op >> den;
			if (op != ':') return false;
		}
		break; case 'X': {
			std::string field;
			is >> field;
		}
		default: {
			return false;
			break;
		}
		}
	}
	return true;
}


bool read_header_frame(std::istream& is) {
	while (1) {
		char c = is.get();
		if (c == '\n') return true; //fine header
		if (c != ' ') return false;
		switch (is.get())
		{
		break; case 'I': {
			std::string s;
			is >> s;
		}
		break; case 'X': {
			std::string field;
			is >> field;
		}
		default: {
			return false;
			break;
		}
		}
	}
	return true;
}
 
uint8_t saturate_Y(uint8_t& val) {
	if (val > 235) {
		return 235;
	}
	else if (val < 16) {
		return 16;
	}
	else return val;
}


uint8_t saturate_C(const uint8_t& val) {
	if (val > 240) {
		return 240;
	}
	else if (val < 16) {
		return 16;
	}
	else return val;
}

void saturate_RGB(int& val) {
	if (val > 255) {
		val = 255;
	}
	else if (val < 0) {
		val = 0;
	}
	else return;
}

bool y4m_extract_color(const std::string& filename, std::vector<mat<vec3b>>& frames) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	int h, w;
	if (!read_header(is, h, w)) return false;
	mat<uint8_t> Y(h, w);
	mat<uint8_t> Cb(h / 2, w / 2);
	mat<uint8_t> Cr(h / 2, w / 2);
	mat<vec3b> rgb(h, w);
	std::string magic_number;
	while (is >> magic_number) {
		if (magic_number != "FRAME") return false;
		//frame header
		if (!read_header_frame(is)) {
			return false;
		}
		//leggo il frame
		is.read(Y.rawdata(), Y.rawsize());
		is.read(Cb.rawdata(), Cb.rawsize());
		is.read(Cr.rawdata(), Cr.rawsize());

		for (int r = 0; r < Y.rows(); ++r) {
			for (int c = 0; c < Y.cols(); ++c) {
				uint8_t y = saturate_Y(Y(r, c));
				uint8_t cb = saturate_C(Cb(r / 2, c / 2));
				uint8_t cr = saturate_C(Cr(r / 2, c / 2));

				int red = 1.164 * (y - 16) + 1.596 * (cr - 128);
				int green = 1.164 * (y - 16) - 0.392 * (cb - 128) - 0.813 * (cr - 128);
				int blue = 1.164 * (y - 16) + 2.017 * (cb - 128);

				saturate_RGB(red);
				saturate_RGB(green);
				saturate_RGB(blue);
				
				rgb(r, c)[0] = (uint8_t)red;
				rgb(r, c)[1] = (uint8_t)green;
				rgb(r, c)[2] = (uint8_t)blue;
			}
		}
		frames.push_back(rgb);
	}
	return true;
}



/*
int main() {
	std::vector<mat<vec3b>> frames;
	bool b = y4m_extract_color("720p_stockholm_ter.y4m", frames);
	for (int i = 0; i < frames.size(); ++i) {
		std::string nome = "frame" + std::to_string(i) + ".ppm";
		save_ppm(nome, frames[i]);
	}
	return 0;
}*/