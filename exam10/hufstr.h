#ifndef HUFSTR_H
#define HUFSTR_H
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

struct elem {
	uint8_t _sym;
	uint8_t _len = 0;
	uint32_t _code = 0;
	elem() {}
	elem(const uint8_t& sym) : _sym(sym) {}
	bool operator<(const elem& rhs) const {
		if (_len < rhs._len)
			return true;
		else if (_len > rhs._len)
			return false;
		else
			return _sym < rhs._sym;
	}
};

class hufstr {
public:
	hufstr();
    std::vector<uint8_t> compress(const std::string& s) const;
    std::string decompress(const std::vector<uint8_t>& v) const;
	std::vector<elem> table;

};

#endif // !HUFSTR_H

