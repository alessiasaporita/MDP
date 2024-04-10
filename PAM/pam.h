#ifndef PAM_H
#define PAM_H

#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdlib.h>


template <typename T>
struct mat {
	int H, W;
	int max_val;
	std::vector<T> data;
};

bool read_PAM(std::istream& is, mat<uint8_t>& m) {
	std::string stringa;
	char carattere;
	int H, W, C, maxval;

	is >> stringa;
	if (stringa != "P7") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	is >> stringa;
	if (stringa != "WIDTH") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return false;

	is >> W;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	is >> stringa;
	if (stringa != "HEIGHT") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return false;

	is >> H;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	is >> stringa;
	if (stringa != "DEPTH") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return false;

	is >> C;
	if (C != 1) return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	is >> stringa;
	if (stringa != "MAXVAL") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return false;

	is >> maxval;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	is >> stringa;
	if (stringa != "TUPLTYPE") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return false;

	is >> stringa;
	if (stringa != "GRAYSCALE") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	is >> stringa;
	if (stringa != "ENDHDR") return false;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return false;

	m.H = H;
	m.W = W;
	m.max_val = maxval;
	m.data.resize(H * W);

	if (!is.read(reinterpret_cast<char*>(&m.data[0]), H * W)) return false;

	return true;
}

bool write_PAM(std::ostream& os, mat<uint8_t>& m) {
	char carattere1 = 0x0A;
	char carattere2 = ' ';
	os << "P7";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);


	os << "WIDTH";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << m.W;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "HEIGHT";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << m.H;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "DEPTH";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << 1;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "MAXVAL";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << m.max_val;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "TUPLTYPE";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << "GRAYSCALE";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "ENDHDR";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os.write(reinterpret_cast<const char*>(&m.data[0]), m.H * m.W);

	return true;
}

#endif // !PAM_H


