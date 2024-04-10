#include "ppm.h"
#include "mat.h"
#include <string>
#include <cstdint>
#include <string>
#include <fstream>
#include <sstream>
/*
template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size=sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	int n_ = 0;
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
	bitreader(std::istream& is): is_(is){}

	uint32_t read(int n) {
		uint32_t val = 0;
		while (n-- > 0) {
			val = (val << 1) | (read_bit() & 1);
		}
		return val;
	}
	uint32_t operator()(int n) {
		return read(n);
	}
	bool fail() const {
		return is_.fail();
	}
	explicit operator bool() const {
		return !fail();
	}
};

void PackBitsEncode(const mat<uint8_t>& img, std::vector<uint8_t>& encoded) {
	int status = -1; //1=copy, 2=run, 0=un carattere letto, -1=nessun carattere letto
	int lrun = 0; //lunghezza run
	uint8_t tmp;
	std::vector<uint8_t> copy;

	for (int r = 0; r < img.rows(); ++r) {
		for (int c = 0; c < img.cols(); ++c) {
			if (status == -1) {
				tmp = img(r, c);
				status = 0;
				continue;
			}
			else {
				if (tmp == img(r, c)) { // tmp == img(r, c)
					if (status == 2) { //sono in una run
						lrun++;
						tmp = img(r, c);
					}
					else{ //prima in una copy o un carattere letto
						if (status == 0) {
							status = 2;
							lrun = 2;
						}
						if (status == 1) { //prima in una copy
							copy.pop_back();
							encoded.push_back(copy.size() - 1);
							for (auto& x : copy) {
								encoded.push_back(x);
							}
							copy.resize(0);
							status = 2;
							lrun = 2;
						}
						tmp = img(r, c);
					}
				}
				else { //tmp != img(r, c)
					if (status == 2) { //prima in una run
						encoded.push_back(257 - lrun);
						encoded.push_back(tmp);
						lrun = 0;
						status = 0;
						tmp = img(r, c);
					}
					else { //prima in una copy o un carattere letto
						if (status == 0) {
							copy.push_back(tmp);
							status = 1;
						}
						copy.push_back(img(r, c));
						tmp = img(r, c);
					}
				}
			}
		}
	}
	if (lrun != 0) {
		encoded.push_back(257 - lrun);
		encoded.push_back(tmp);
	}
	if (copy.size() != 0) {
		encoded.push_back(copy.size() - 1);
		for (auto& x : copy) {
			encoded.push_back(x);
		}
	}
	encoded.push_back(128);
}

uint8_t convert(uint8_t val) {
	if (val >= 0 && val <= 25) {
		return 'A' + val;
	}
	else if (val >= 26 && val <= 51) {
		return 'a' + val - 26;
	}
	else if (val >= 52 && val <= 61) {
		return '0' + val - 52;
	}
	else if (val == 62) return '+';
	else if (val == 63) return '/';
	else return 0;
}

std::string Base64Encode(const std::vector<uint8_t>& v) {
	std::string base64;
	uint8_t val;
	std::stringstream ss;
	ss.write(reinterpret_cast<const char*>(&v[0]), v.size());
	bitreader br(ss);
	for (size_t i = 0; i < v.size(); ++i) {
		base64.push_back(convert(br(6)));
		base64.push_back(convert(br(6)));
		base64.push_back(convert(br(6)));
		base64.push_back(convert(br(6)));
		i += 2;
	}
	return base64;
}
/*
int main() {
	std::vector<uint8_t> encoded = { 0xfe, 0x00, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0xff, 0xfe, 0x00, 0xfe, 0xff, 0x80 };
	std::string base64;
	base64 = Base64Encode(encoded);
	return 0;
}*/
