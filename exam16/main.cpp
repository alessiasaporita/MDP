#include <fstream>
#include <stdlib.h>
#include <vector>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size=sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}


void read_data(std::istream& is, size_t n, std::vector<uint8_t>& buffer) {
	uint8_t byte;
	for (size_t i = 0; i < n; ++i) {
		raw_read(is, byte);
		buffer.push_back(byte);
	}
}

void write_data(std::ostream& os, std::vector<uint8_t>& buffer) {
	for (size_t i = 0; i < buffer.size(); ++i) {
		os.write(reinterpret_cast<const char*>(&buffer[i]), 1);
	}
}

int lz4decomp(std::istream& is, std::ostream& os) {
	uint32_t magic_number;
	uint32_t length; //lunghezza del file compresso
	uint32_t const_value;
	raw_read(is, magic_number);
	if (magic_number != 0x184C2103) return 1;
	raw_read(is, length);
	raw_read(is, const_value);
	if (const_value != 0x4D000000) return 1;

	uint32_t l_block = 0; //lunghezza blocco
	uint8_t token;
	uint32_t l_literals; //lunghezza literals
	uint16_t offset;
	uint32_t matchlength;

	uint8_t byte;
	std::vector<uint8_t> dict;
	uint32_t dim = 0; //byte del block letti

	while (1) {
		if (dim >= l_block) { //inizio di un blocco
			dim = 0;
			if (!raw_read(is, l_block)) break; //lunghezza blocco
		}
		//sequenza
		raw_read(is, token); //token
		dim++;
		l_literals = (token >> 4) & 0x0F;
		matchlength = token & 0x0F;
		if (l_literals == 15) {
			raw_read(is, byte);
			dim++;
			l_literals += byte;
			while (1) {
				if (byte == 255) {
					raw_read(is, byte);
					dim++;
					l_literals += byte;
				}
				else break;
			}
		}
		read_data(is, l_literals, dict); //leggo l_literals byte
		dim += l_literals;

		if (l_block - dim > 5) { //calcolo offset e matchlength e copio i byte
			raw_read(is, offset); //offset
			dim += 2;
			matchlength += 4;
			if (matchlength == 19) {
				raw_read(is, byte);
				dim++;
				matchlength += byte;
				while (1) {
					if (byte == 255) {
						raw_read(is, byte);
						dim++;
						matchlength += byte;
					}
					else break;
				}
			}
			for (size_t i = 0; i < matchlength; ++i) {
				dict.push_back(dict[dict.size() - size_t(offset)]);
			}
		}
	}
	write_data(os, dict);
	return 0;
}


int main (int argc, char *argv[]){
	if (argc != 3) return 1;

	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return 1;

	std::ofstream os(argv[2], std::ios_base::binary);
	if (!os) return 1;

	return lz4decomp(is, os);
}