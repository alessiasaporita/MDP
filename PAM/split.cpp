/*
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

int main(int argc, char *argv[]) {
	if(argc!=2){
		return 1;
	}
	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return 1;

	std::string filename = argv[1];
	std::string nome = filename.substr(0, filename.find('.'));
	std::ofstream os_R(nome + "_R.pam", std::ios_base::binary);
	if (!os_R) return 1;
	std::ofstream os_G(nome + "_G.pam", std::ios_base::binary);
	if (!os_G) return 1;
	std::ofstream os_B(nome + "_B.pam", std::ios_base::binary);
	if (!os_B) return 1;

	std::vector<std::ostream*> out_files;
	out_files.push_back(&os_R);
	out_files.push_back(&os_G);
	out_files.push_back(&os_B);

	std::string stringa;
	char carattere;
	int H, W, C, maxval;

	is >> stringa;
	if (stringa != "P7") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	
	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}
	
	is >> stringa;
	if (stringa != "WIDTH") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return 1;

	
	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}

	is >> W;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	for (auto& x : out_files) {
		*x << W;
		*x << carattere;
	}

	is >> stringa;
	if (stringa != "HEIGHT") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return 1;


	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}
	is >> H;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	for (auto& x : out_files) {
		*x << H;
		*x << carattere;
	}
	
	is >> stringa;
	if (stringa != "DEPTH") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return 1;

	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}

	is >> C;
	if (C != 3) return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	for (auto& x : out_files) {
		*x << 1;
		*x << carattere;
	}

	is >> stringa;
	if (stringa != "MAXVAL") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return 1;
	
	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}
	
	is >> maxval;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;
	
	for (auto& x : out_files) {
		*x << maxval;
		*x << carattere;
	}

	is >> stringa;
	if (stringa != "TUPLTYPE") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != ' ') return 1;
	
	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}

	is >> stringa;
	if (stringa != "RGB") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;
	
	for (auto& x : out_files) {
		*x << "GRAYSCALE";
		*x << carattere;
	}

	is >> stringa;
	if (stringa != "ENDHDR") return 1;
	is.read(reinterpret_cast<char*>(&carattere), 1);
	if (carattere != 0x0A) return 1;

	
	for (auto& x : out_files) {
		*x << stringa;
		*x << carattere;
	}
	/*
	uint8_t elem;
	while (is.read(reinterpret_cast<char*>(&elem), 1)) {
		*out_files[0] << elem;
		is.read(reinterpret_cast<char*>(&elem), 1);
		*out_files[1] << elem;
		is.read(reinterpret_cast<char*>(&elem), 1);
		*out_files[2] << elem;
	}
	
	std::vector<uint8_t> edata;
	edata.resize(H * W * 3);
	is.read(reinterpret_cast<char*>(&edata[0]), H * W * 3);
	for (int i = 0; i < edata.size(); ++i) {		
		*out_files[0] << edata[i];
		*out_files[1] << edata[i+1];
		*out_files[2] << edata[i+2];
		i += 2;
	}
	return 0;
}*/