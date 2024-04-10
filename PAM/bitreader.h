#ifndef  BITREADER_H
#define BITREADER_H
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdlib.h>

template<typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	std::istream& is_;
	uint8_t n_ = 0;

	uint8_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_);
	}
public:
	bitreader(std::istream& is) : is_(is) {}

	uint32_t read(uint8_t n) {
		uint32_t val = 0;
		while (n-- > 0) {
			val = (val << 1) | (read_bit() & 1);
		}
		//int i = static_cast<int>(val);
		return val;
	}

	uint32_t operator()(uint8_t n) {
		return read(n);
	}

	bool fail() const {
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};


#endif // ! BITREADER_H