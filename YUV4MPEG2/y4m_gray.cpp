#include <string>
#include <vector>
#include "mat.h"
#include <fstream>
/*
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

bool y4m_extract_gray(const std::string& filename, std::vector<mat<uint8_t>>& frames) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	int h, w;
	if (!read_header(is, h, w)) return false;
	mat<uint8_t> Y(h, w);
	mat<uint8_t> Cb(h / 2, w / 2);
	mat<uint8_t> Cr(h / 2, w / 2);
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
		frames.push_back(Y);
	}
	return true;
}*/
