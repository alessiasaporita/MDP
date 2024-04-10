#include <stdlib.h>
#include <cstring>
#include <vector>
#include <fstream>
#include "mat.h"
#include "pgm.h"
#include <sstream>
#include <string>
#include <cassert>
#include <cstdint>
#include <array>
#include <iostream>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}



bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	
	std::string stream_header;
	is >> stream_header;
	if (stream_header != "YUV4MPEG2") return false;

	char character;
	int h = -1, w = -1;

	while (raw_read(is, character)) {
		if (character == '\n') break;
		else if (character == ' ') continue;
		switch (character) {
			case 'H': {
				is >> h;
				continue;
			}
			case 'W': {
				is >> w;
				continue;
			}
			case 'C': {
				std::string chroma;
				is >> chroma;
				if (chroma != "420jpeg") return false;
				continue;
				}
			case 'I': {
				char p;
				is >> p;
				if (p != 'p') return false;
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
				std::string a_field;
				is >> a_field;
				continue;
			}
			default: return false;
		}
		return false;
	}

	if (w < 0 || h < 0) return false;

	std::string frame_header;
	mat<uint8_t> Y(h, w);
	mat<uint8_t> Cb(h / 2, w / 2);
	mat<uint8_t> Cr(h / 2, w / 2);

	while (is >> frame_header) {
		if (frame_header != "FRAME")
			return false;

		//leggo tutti i caratteri fino al primo \n
		while (raw_read(is, character)) {
			if (character == '\n') break;
			if (character == ' ') continue;
			switch (character) {
			case 'I': {
				char p;
				is >> p;
				if (p != 'p') return false;
				continue;
			}
			case 'X': {
				std::string a_field;
				is >> a_field;
				continue;
			}
			default: return false;
			}
			return false;
		}

		is.read(Y.rawdata(), h * w);
		is.read(Cb.rawdata(), h * w / 4);
		is.read(Cr.rawdata(), h * w / 4);
	
		frames.push_back(Y);
	}
	return true;
}


/*
int main() {
	std::vector<mat<uint8_t>> frames;
	if (y4m_extract_gray("foreman_cif.y4m", frames)) {
		for (size_t i = 0; i < frames.size(); ++i) {
			std::stringstream ss;
			ss << "frame" << i << ".pgm";
			save_pgm(ss.str(), frames[i]);
		}
	}
	return 0;
}
*/

