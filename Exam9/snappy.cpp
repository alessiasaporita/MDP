#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
/*
struct timer {
	std::chrono::steady_clock::time_point start_;
	timer() {
		start();
	}
	void start() {
		start_ = std::chrono::steady_clock::now();
	}
	auto elapsed_ms() {
		using namespace std::chrono;
		auto stop = steady_clock::now();
		return duration_cast<milliseconds>(stop - start_);
	}
};
*/
template <typename T>
std::istream &raw_read(std::istream &is, T &val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char *>(&val), size);
}

template <typename T>
std::ostream &raw_write(std::ostream &os, T &val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char *>(&val), size);
}
int main(int argc, char *argv[]) 
{
	/*
	using namespace std::chrono;
	timer t;
	t.start();
	*/
	if (argc != 3) return 1;

	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return 1;

	std::ofstream os(argv[2], std::ios_base::binary);
	if (!os) return 1;

	uint8_t bit = 0;
	uint8_t byte;
	uint64_t lenght = 0; //lunghezza del file
	uint8_t len = 0;  //lunghezza copy
	int64_t offset = 0; //offset copy
	uint64_t count = 0; //lunghezza letta
	uint64_t preamble;
	int i = 0;

	//preambolo
	while (1) {
		raw_read(is, preamble, 1);
		bit = (preamble >> 7) & 1; //estraggo bit più significativo
		preamble = preamble & 0x7F; //rimuovo il bit più significativo
		preamble = preamble << (i * 7); //shifto i bit
		lenght = lenght | preamble; //aggiungo i 7 bit alla lunghezza.
		++i;
		if (bit == 0) break;
	}

	std::vector<uint8_t> dict;

	//testo
	while (raw_read(is, byte)) {
		if (dict.size() > 100) {
			int x = 7;
		}
		bit = byte & 0x03; //tipo
		if (bit == 0) { //literal
			len = (byte >> 2) & 0x3F; //len - 1
			len += 1;
			if (len >= 0 && len <= 60) { //lunghezza del literal 
				preamble = len;
			}
			else {
				len = len - 60; //devo leggere len byte
				preamble = 0;
				if (!raw_read(is, preamble, len)) return 1;
				preamble += 1;
			}
			//leggo e scrivo preamble byte
			for (size_t i = 0; i < preamble; ++i) {
				if (!raw_read(is, byte)) return 1;
				//raw_write(os, byte);
				dict.push_back(byte);
			}//

			// Alternativa
			/*
			const auto block_size = 4096;
			while (preamble >= block_size) {
				auto pos = dict.size();
				dict.resize(pos + block_size);
				raw_read(is, dict[pos], block_size);
				preamble -= block_size;
			}
			if (preamble > 0) {
				auto pos = dict.size();
				dict.resize(pos + size_t(preamble));
				raw_read(is, dict[pos], size_t(preamble));
			}
			*/
		}
		else {	//copy
			if (bit == 1) { // Copy with 1-byte offset
				len = (byte >> 2) & 0x07; //len - 4
				len += 4;
				offset = byte & 0XE0;
				offset = offset << 3;
				if (!raw_read(is, byte)) return 1;
				offset += byte;
			}
			else {
				len = (byte >> 2) & 0X3F; //len-1
				len += 1;
				offset = 0;
				if (bit == 2) {
					if (!raw_read(is, offset, 2)) return 1;
				}// Copy with 2-byte offset
				else {
					if (!raw_read(is, offset, 4)) return 1;
				}//copy with 4-byte offset
			}
			for (size_t i = 0; i < len; ++i) {
				dict.push_back(dict[dict.size() - size_t(offset)]);
			}
		}
	}
	raw_write(os, dict[0], dict.size());
	/*
	auto elapsed = t.elapsed_ms();
	std::cout << "Total time: " << elapsed << "\n";
	*/
}
