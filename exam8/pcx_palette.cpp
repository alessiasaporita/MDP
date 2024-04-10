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

bool load_pcx(const std::string& filename, mat<vec3b>& img) {
	std::ifstream is(filename, std::ios_base::binary);
	if (!is) return false;
	header h;
	read_header(is, h);
	img.resize(h.Ymax - h.Ymin + 1, h.Xmax - h.Xmin + 1);
	uint32_t totalbytes = h.colorplanes * h.bytesperplaneline;
	uint8_t byte;
	std::vector<uint8_t> row;
	std::map<uint8_t, vec3b> palette;
	mat<uint8_t> mappa(img.rows(), img.cols());
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
		for (int c = 0; c < img.cols(); ++c) {
			mappa(r, c) = row[c];
		}
		row.resize(0);
		r++;
	}

	save_pgm("mappa.pgm", mappa); //salvo la mappa

	//leggo la palette --> 256 triplets of bytes
	vec3b pixel;
	byte = is.get();
	if (byte != 12) return false;
	for (size_t i = 0; i < 256; ++i) {
		pixel[0] = is.get();
		pixel[1] = is.get();
		pixel[2] = is.get();
		palette.insert({ i, pixel });
	}

	//inserisco i valori nella matrice
	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			auto x = palette.find(mappa(r, c));
			if (x == palette.end()) {
				return false;
			}
			img(r, c) = x->second;
		}
	}
	return true;

}