#include "pcx.h"
#include <fstream>
#include <cstdint>
#include "utils.h"
#include "pgm.h"
#include <vector>
#include <cstdio>
#include <map>

struct header {
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bitsperplane;
	uint16_t Xmin;
	uint16_t Ymin;
	uint16_t Xmax;
	uint16_t Ymax;
	uint16_t vertDPI;
	uint16_t horzDPI;
	uint8_t palette[48];
	uint8_t reserved;
	uint8_t colorplanes;
	uint16_t bytesperplaneline;
	uint16_t paletteInfo;
	uint16_t horScrSize;
	uint16_t verScrSize;
	uint8_t padding[54];
};

bool read_header(std::istream& is, header& h) {
	h.manufacturer = is.get();
	if (h.manufacturer != 0x0A) return false;
	h.version = is.get();
	if (h.version != 5) return false;
	h.encoding = is.get();
	h.bitsperplane = is.get();
	raw_read(is, h.Xmin);
	raw_read(is, h.Ymin);
	raw_read(is, h.Xmax);
	raw_read(is, h.Ymax);
	raw_read(is, h.vertDPI);
	raw_read(is, h.horzDPI);
	raw_read(is, h.palette);
	h.reserved = is.get();
	h.colorplanes = is.get();
	raw_read(is, h.bytesperplaneline);
	raw_read(is, h.paletteInfo);
	raw_read(is, h.horScrSize);
	raw_read(is, h.verScrSize);
	raw_read(is, h.padding);
	return true;
}

bool load_pcx(const std::string& filename, mat<vec3b>& img){
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	header h;
	read_header(is, h);
	img.resize(h.Ymax - h.Ymin + 1, h.Xmax - h.Xmin + 1);
	uint32_t totalbytes = h.colorplanes * h.bytesperplaneline;
	uint8_t byte;
	std::vector<uint8_t> row;
	int r = 0; 
	
	while (r < img.rows()) {
		//leggo una riga della matrice
		while (row.size() < totalbytes) {
			byte = is.get();
			if (((byte >> 6) & 0b00000011) == 3) { // run
				uint8_t len = byte & 0b00111111;
				byte = is.get();
				//copio len volte il prossimo byte nella img
				for (size_t j = 0; j < len; ++j) {
					row.push_back(byte);
				}
			}
			else { // data
				row.push_back(byte);
			}
		}
		//inserisco la riga nella matrice
		vec3b pixel;
		for (int c = 0; c < img.cols(); ++c) {
			pixel[0] = row[c]; //[0, h.bytesperplaneline - 1]
			pixel[1] = row[h.bytesperplaneline + c]; //[h.bytesperplaneline, 2 * h.bytesperplaneline - 1]
			pixel[2] = row[2 * h.bytesperplaneline + c]; //[2 * h.bytesperplaneline, 3 * h.bytesperplaneline - 1]
			img(r, c) = pixel;
		}
		row.resize(0);
		r++;
	}
	return true;
}

