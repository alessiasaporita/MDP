/*
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

template<typename T>
struct mat{
	int H, W;
	int max_val;
	std::vector<T> data;
};

struct vect3b {
	uint8_t r, g, b;
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

	if(!is.read(reinterpret_cast<char*>(&m.data[0]), H * W)) return false;

	return true;
}

bool write_PAM_RGB(std::ostream& os, mat<vect3b>& rgb) {
	char carattere1 = 0x0A;
	char carattere2 = ' ';
	os << "P7";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);
	

	os << "WIDTH";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << rgb.W;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);
	
	os << "HEIGHT";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << rgb.H;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "DEPTH";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << 3;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "MAXVAL";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << rgb.max_val;
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "TUPLTYPE";
	os.write(reinterpret_cast<const char*>(&carattere2), 1);

	os << "RGB";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);

	os << "ENDHDR";
	os.write(reinterpret_cast<const char*>(&carattere1), 1);
	
	vect3b elem; 
	for (int i = 0; i < rgb.data.size(); ++i) {
		elem = rgb.data[i];
		os.write(reinterpret_cast<const char*>(&elem.r), 1);
		os.write(reinterpret_cast<const char*>(&elem.g), 1);
		os.write(reinterpret_cast<const char*>(&elem.b), 1);
	}

	return true;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		return 1;
	}
	
	std::string name = argv[1];

	std::ifstream is_R(name + "_R.pam", std::ios_base::binary);
	if (!is_R) return 1;
	std::ifstream is_G(name + "_G.pam", std::ios_base::binary);
	if (!is_G) return 1;
	std::ifstream is_B(name + "_B.pam", std::ios_base::binary);
	if (!is_B) return 1;

	mat<uint8_t> R, G, B;
	if (!read_PAM(is_R, R)) return 1;
	if (!read_PAM(is_G, G)) return 1;
	if (!read_PAM(is_B, B)) return 1;
	if (R.H != G.H || R.W != G.W || R.H != B.H || R.W != B.W || B.H != G.H || B.W != G.W) return 1;

	std::ofstream os(name + "_reconstructed.pam", std::ios_base::binary);
	if (!os) return 1;

	mat<vect3b> rgb;
	rgb.H = R.H;
	rgb.W = R.W;
	rgb.max_val = R.max_val;
	vect3b elem;

	for (size_t i = 0; i < R.data.size(); ++i) {
		elem.r = R.data[i];
		elem.g = G.data[i];
		elem.b = B.data[i];
		rgb.data.push_back(elem);
	}
	write_PAM_RGB(os, rgb);
	return 0;
}*/