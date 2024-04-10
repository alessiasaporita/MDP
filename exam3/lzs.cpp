#include "lzs.h"
#include <map>
#include <string>
#include <cstdint>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size= sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	int n_;
	std::istream& is_;

	uint8_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::istream& is): is_(is), n_(0) {}

	uint32_t read(uint8_t n) {
		uint32_t val = 0;
		while (n-- > 0) {
			val = (val << 1) | read_bit();
		}
		return val;
	}

	uint32_t operator() (uint8_t n) {
		return read(n);
	}

	bool fail() const{
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};

void Huffman_Table(std::map<uint8_t, uint8_t>& table) {
	table.insert({ 0, 2 });
	table.insert({ 1, 3 });
	table.insert({ 2, 4 });
	table.insert({ 12, 5 });
	table.insert({ 13, 6 });
	table.insert({ 14, 7 });
}

void lzs_decompress(std::istream& is, std::ostream& os) {
	std::string output;
	bitreader br(is);
	std::map<uint8_t, uint8_t> table;
	uint8_t bit;
	uint8_t character;
	uint16_t offset;
	uint32_t length = 0;
	uint16_t end_marker = 0;
	int N = 0;

	Huffman_Table(table); //creo tabella di Huffman
	while (1) {
		bit = br(1);
		if (!br) {
			return; //lettura fallita
		}
		if (bit == 0) { //literal
			character = br(8);
			output.push_back(character);
		}
		else { //Offset/length, bit=1

			//OFFSET
			bit = br(1);
			end_marker = 0;

			if (bit == 0) {
				offset = br(11);
			}
			else { //può essere offset o end marker
				end_marker = (end_marker | bit) << 7;
				offset = br(7);
				end_marker += offset;
				end_marker = end_marker << 1;
			}
			bit = br(1);
			end_marker += bit;
			if (end_marker == 256) {//endmarker
				os.write(reinterpret_cast<const char*>(&output[0]), output.size());
				return;
			}

			//LENGTH
			bit = (bit << 1) | br(1);
			auto search = table.find(bit);
			if (search != table.end()) { //trovato il codice a due bit
				length = search->second;
			}
			else { //leggo altri due bit
				//3 --> 15 |--> 15 - 3 = 12
				bit = (bit << 2) & (0b00000011 + 12);
				bit = bit | br(2);
				auto search = table.find(bit);
				if (search != table.end()) { //trovato il codice a 4 bit
					length = search->second;
				}
				else if (bit == 0b00001111) { //length > 7
					//++N;
					while (bit == 15) {
						bit = br(4);
						++N;
					}
					//xxxx=bit
					length = bit + 15 * N - 7;
					N = 0;
				}
			}

			//aggiungo i caratteri
			for (size_t i = 0; i < length; ++i) {
				output.push_back(output[output.size() - offset]);
			}
		}
	}
}

/*
int main() {
	std::ifstream is("bibbia.txt.lzs", std::ios_base::binary);
	if (!is) return 1;
	std::ofstream os("output_bibbia.txt", std::ios_base::binary);
	if (!os) return 1;
	lzs_decompress(is, os);
	return 0;
}*/
