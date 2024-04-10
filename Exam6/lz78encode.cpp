#include <stdlib.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>

template <typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

class bitwriter {
	int n_;
	uint8_t buffer_;
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
	bitwriter(std::ostream& os): n_(0), os_(os){}

	std::ostream& write(uint32_t val, uint8_t n) {
		for (int i = n - 1; i >= 0; --i) {
			write_bit(val >> i);
		}
		return os_;
	}

	std::ostream& operator() (uint32_t val, uint8_t n) {
		write(val, n);
		return os_;
	}

	std::ostream& flush() {
		while (n_ > 0) {
			write_bit(0);
		}
		return os_;
	}

	~bitwriter() {
		flush();
	}
};

bool lz78encode(const std::string& input_filename, const std::string& output_filename, int maxbits) {
	std::ifstream is(input_filename, std::ios_base::binary);
	if (!is) return false;

	std::ofstream os(output_filename, std::ios_base::binary);
	if (!os) return false;

	if (maxbits < 1 or maxbits>30) return false;
	std::map<std::vector<char>, int> dictionary;
	std::map<std::vector<char>, int>::iterator it;
	bitwriter bw(os);
	std::vector<char> index(0);
	std::vector<char> caratteri(0);
	char c;

	os.write(reinterpret_cast<const char*>("LZ78"), 4);
	bw(maxbits, 5);
	
	while (is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
		caratteri.push_back(c);
		if (dictionary.size() == 0) {
			dictionary.insert(std::pair<std::vector<char>, int>(caratteri, 1));
			bw(c, 8);
			caratteri.resize(0);
			continue;
		}
		it = dictionary.find(caratteri);
		if (it != dictionary.end()) { // trovo la stringa
			index = dictionary.find(caratteri)->first;
			continue;
		}
		else{
			if (index.size() == 0) {
				bw(0, std::log2(dictionary.size()) + 1);
				dictionary.insert(std::pair<std::vector<char>, int>(caratteri, dictionary.size()+1));
				bw(c, 8);
				caratteri.resize(0);
			}
			else {
				bw(dictionary.find(index)->second, std::log2(dictionary.size())+1);
				bw(c, 8);
				index.push_back(c);
				dictionary.insert(std::pair<std::vector<char>, int>(index, dictionary.size()+1));
				index.resize(0);
				caratteri.resize(0);
			}
			if (dictionary.size() == std::pow(2, maxbits)) { //svuoto dizionario
				dictionary.clear(); 
			}
		}
	}
	if (caratteri.size() != 0) {
		caratteri.pop_back();
		it = dictionary.find(caratteri);
		if (it != dictionary.end()) { // trovo la stringa
			index = dictionary.find(caratteri)->first;
			bw(dictionary.find(index)->second, std::log2(dictionary.size()) + 1);
			bw(c, 8);
		}
		else {
			bw(0, std::log2(dictionary.size()) + 1);
			bw(c, 8);
		}
	}
	return true;
}

int main(int arc, char *argv[]) {
	int n = std::stoi(argv[3]);
	bool risp = lz78encode(argv[1], argv[2], n);
	if (risp) return 0;
	else return 1;
}