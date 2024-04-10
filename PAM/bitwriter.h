#ifndef BITWRITER_H
#define BITWRITER_H
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdlib.h>

template<typename T>
std::ostream& raw_write(std::ostream& os, T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
	std::ostream& os_;
	uint8_t buffer_;
	uint8_t n_;

	std::ostream& write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		n_++;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
		return os_;
	}
public:
	bitwriter(std::ostream& os) : os_(os), n_(0) {}

	std::ostream& write(uint32_t val, uint8_t n) {
		for (int i = n - 1; i >= 0; --i) {
			write_bit(val >> i);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t val, uint8_t n) {
		return write(val, n);
	}

	void flush() {
		while (n_ > 0) {
			write_bit(0);
		}
	}

	~bitwriter() {
		flush();
	}
};



#endif // !BITWRITER_H

