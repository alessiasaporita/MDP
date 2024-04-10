#include "hufstr.h"
#include <sstream>

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}
template<typename T>
std::ostream& raw_write(std::ostream& os, T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
	uint8_t buffer_;
	int n_;
	std::ostream& os_;

	std::ostream& write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			raw_write(os_, buffer_);
			n_ = 0;
		}
		return os_;
	}
public:
	bitwriter(std::ostream& os) : os_(os), n_(0) {}

	std::ostream& write(uint32_t val, int n) {
		for (int i = n - 1; i >= 0; --i) {
			write_bit(val >> i);
		}
		return os_;
	}

	std::ostream& operator() (uint32_t val, int n) {
		return write(val, n);
	}

	std::ostream& flush(uint32_t bit = 0) {
		while (n_ > 0) {
			write_bit(bit);
		}
		return os_;
	}

	~bitwriter() {
		flush();
	}
};
class bitreader {
	uint8_t buffer_;
	uint8_t n_ = 0;
	std::istream& is_;

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

	uint32_t read(int n) {
		uint32_t val = 0;
		while (n-- > 0) {
			val = (val << 1) | (read_bit() & 1);
		}
		return val;
	}

	uint32_t operator ()(int n) {
		return read(n);
	}

	bool fail() const{
		return is_.fail();
	}

	explicit operator bool() const {
		return !fail();
	}
};

hufstr::hufstr() {
	std::ifstream is("table.bin", std::ios_base::binary);
	elem e;
	bitreader br(is);
	while (1) {
		e._sym = br(8);
		if (!br) {
			break;
		}
		e._len = br(8);
		e._code = br(e._len);
		table.push_back(e);
	}
}


std::vector<uint8_t> hufstr::compress(const std::string& s) const {
	std::vector<uint8_t> cstring;
	//uint8_t buffer;
	//int n = 0;
	std::stringstream stream;
	bitwriter bw(stream);
	for (const auto& x : s) {
		for (const auto& t : table) {
			if (t._sym == x) {
				bw.write(t._code, t._len);
				/*
				for (int i = 0; i < t._len; ++i) {
					uint8_t bit = (t._code >> (t._len - 1 - i));
					buffer = (buffer << 1) | (bit & 1);
					n++;
					if (n == 8) {
						cstring.push_back(buffer);
						n = 0;
					}
				}
				*/
				break;
			}
		}
	}
	/*
	while (n > 0) {
		buffer = (buffer << 1) | 1;
		n++;
		if (n == 8) {
			cstring.push_back(buffer);
			n = 0;
		}
	}*/
	bw.flush(1);
	uint8_t carattere;
	while (stream.read(reinterpret_cast<char*>(&carattere), 1)) {
		cstring.push_back(carattere);
	}
	return cstring;
}


std::string hufstr::decompress(const std::vector<uint8_t>& v) const {
	std::string dstring;
	int len = 0;
	uint32_t code = 0;
	for (const auto& x : v) {
		for (size_t i = 0; i < 8; ++i) {
			uint8_t bit = (x >> (7 - i));
			code = (code << 1) | (bit & 1);
			len++;
			if (len > table[table.size() - 1]._len) break;
			if (len < table[0]._len) continue;
			for (const auto& t : table) {
				if (t._code == code && t._len == len) {
					dstring.push_back(t._sym);
					len = 0;
					code = 0;
					break;
				}
			}

		}
	}
	return dstring;

}

/*
int main() {

	hufstr h;
	std::vector<uint8_t> c = h.compress("aeio");
	std::string s = h.decompress(c);
	int mio = 0;
	return 0;
}*/