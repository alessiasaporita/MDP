#include "pcx.h"
#include "utils.h"
#include <cstdint>
#include <fstream>
#include "pgm.h"
#include <map>
//#include "ppm.h"

struct header {
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bitsperplane;
	uint16_t windowXmin;
	uint16_t windowYmin;
	uint16_t windowXmax;
	uint16_t windowYmax;
	uint16_t vertDPI;
	uint16_t horzDPI;
	uint8_t palette[48];
	uint8_t reserved;
	uint8_t colorplanes;
	uint16_t bytesperplaneline;
	uint16_t paletteinfo;
	uint16_t hoscrsize;
	uint16_t verscrsize;
	uint8_t padding[54];
};

bool read_header(std::istream& is, header& h) {
	raw_read(is, h.manufacturer);
	if (h.manufacturer != 0x0A) return false;
	raw_read(is, h.version);
	if (h.version != 5) return false;
	raw_read(is, h.encoding);
	raw_read(is, h.bitsperplane);
	raw_read(is, h.windowXmin);
	raw_read(is, h.windowYmin);
	raw_read(is, h.windowXmax);
	raw_read(is, h.windowYmax);
	raw_read(is, h.vertDPI);
	raw_read(is, h.horzDPI);
	raw_read(is, h.palette);
	raw_read(is, h.reserved);
	raw_read(is, h.colorplanes);
	raw_read(is, h.bytesperplaneline);
	raw_read(is, h.paletteinfo);
	raw_read(is, h.hoscrsize);
	raw_read(is, h.verscrsize);
	raw_read(is, h.padding);
	return true;
}

//If a bit is 0, the corresponding pixel shall have value 0, if a bit is 1, the corresponding pixel shall have value 255.
bool load_pcx(const std::string& filename, mat<uint8_t>& img) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;

	header h;
	if (!read_header(is, h)) return false;
	uint32_t x = h.windowXmax - h.windowXmin + 1;
	uint32_t y = h.windowYmax - h.windowYmin + 1;
	img.resize(y, x);
	uint32_t totalbytes = h.bytesperplaneline * h.colorplanes;
	uint32_t len;
	std::vector<uint8_t> data;
	int r = 0;
	while (r < img.rows()) {
		while (data.size() < totalbytes) { //leggo una scan line
			uint8_t byte = is.get();
			if (!is) break;
			if (((byte >> 6) & 0b0000'0011) == 0b0000'0011) { //run-length
				len = byte & 0b0011'1111;
				byte = is.get();
				for (uint32_t i = 0; i < len; ++i) {
					data.push_back(byte);
				}
			}
			else {
				data.push_back(byte);
			}
		}
		int index = 0;
		//inserisco la riga
		for (int c = 0; c < img.cols(); ++c) {
			uint8_t byte = data[index];
			img(r, c) = ((byte >> 7) & 0b0000'0001) == 0 ? 0 :  255;
			c++;
			img(r, c) = ((byte >> 6) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = ((byte >> 5) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = ((byte >> 4) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = ((byte >> 3) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = ((byte >> 2) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = ((byte >> 1) & 0b0000'0001) == 0 ? 0 : 255;
			c++;
			img(r, c) = (byte & 0b0000'0001) == 0 ? 0 : 255;
			index++;
		}
		r++;
		data.resize(0);
	}
	return true;
}
/*
bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;

	header h;
	if (!read_header(is, h)) return false;
	uint32_t x = h.windowXmax - h.windowXmin + 1;
	uint32_t y = h.windowYmax - h.windowYmin + 1;
	img.resize(y, x);
	uint32_t totalbytes = h.bytesperplaneline * h.colorplanes;
	uint32_t len;
	std::vector<uint8_t> data;
	int r = 0;
	while (r < img.rows()) {
		while (data.size() < totalbytes) { //leggo una scan line
			uint8_t byte = is.get();
			if (!is) break;
			if (((byte >> 6) & 0b0000'0011) == 0b0000'0011) { //run-length
				len = byte & 0b0011'1111;
				byte = is.get();
				for (uint32_t i = 0; i < len; ++i) {
					data.push_back(byte);
				}
			}
			else {
				data.push_back(byte);
			}
		}
		//inserisco la riga
		for (int c = 0; c < img.cols(); ++c) {
			vec3b pixel;
			pixel[0] = data[c]; //0 -> h.bytesperplaneline - 1
			pixel[1] = data[c + h.bytesperplaneline]; //h.bytesperplaneline - 1 -> 2 * h.bytesperplaneline - 1
			pixel[2] = data[c + 2 * h.bytesperplaneline]; 
			img(r, c) = pixel;
		}
		r++;
		data.resize(0);
	}
	return true;
}*/

bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;

	header h;
	if (!read_header(is, h)) return false;
	uint32_t x = h.windowXmax - h.windowXmin + 1;
	uint32_t y = h.windowYmax - h.windowYmin + 1;
	img.resize(y, x);
	uint32_t totalbytes = h.bytesperplaneline * h.colorplanes;
	uint32_t len;
	std::vector<uint8_t> data;
	int r = 0;
	int count = 0;
	while (r < img.rows()) {
		while (count < totalbytes) { //leggo una scan line
			uint8_t byte = is.get();
			if (!is) break;
			if (((byte >> 6) & 0b0000'0011) == 0b0000'0011) { //run-length
				len = byte & 0b0011'1111;
				count += len;
				byte = is.get();
				if (count <= h.bytesperplaneline) {
					for (uint32_t i = 0; i < len; ++i) {
						data.push_back(byte);
					}
				}
			}
			else {
				count++;
				if (count <= h.bytesperplaneline) {
					data.push_back(byte);
				}
			}
		}
		count = 0;
		r++;
	}
	//palette
	std::map<int, vec3b> palette;
	if (is.get() != 12) return false;
	vec3b pixel;
	int l = 0;
	for (int i = 0; i < 256; ++i) {
		pixel[0] = is.get();
		pixel[1] = is.get();
		pixel[2] = is.get();
		palette[l] = pixel;
		++l;
	}
	int index = 0;
	//costuisco l'immagine con la palette
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			auto val = palette.find(data[index]);
			if (val == palette.end()) return false;
			img(r, c) = val->second;
			++index;
		}
	}
	return true;
}
/*
int main() {
	mat<uint8_t> m;
	mat<vec3b> img;
	load_pcx("islanda_colori_8bit.pcx", img);
	//save_pgm("hose.pgm", m);
	save_ppm("islanda_colori_8bit.ppm", img);
	return 0;

}*/