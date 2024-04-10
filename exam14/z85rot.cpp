#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <array>
#include <stdlib.h>
#include <cmath>

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

using vec3b = std::array<uint8_t, 3>;

template <typename T>
class mat {
	int rows_ = 0, cols_ = 0;
	std::vector<T> data_;

public:
	mat(){}
	mat(int r, int c): rows_(r), cols_(c), data_(r*c) {	}

	int rows() const { return rows_; }
	int cols() const { return cols_; }
	char* raw_data() { return reinterpret_cast<char *>(&data_[0]); }
	const char* raw_data() const{ return reinterpret_cast<const char*>(&data_[0]); }

	void resize(int r, int c) {
		rows_ = r;
		cols_ = c;
		data_.resize(r * c);
	}
	const T& operator()(int r, int c) const{ return data_[r * cols_ + c]; }
	T& operator()(int r, int c) { return data_[r * cols_ + c]; }
};

bool read_header(std::istream& is, mat<vec3b>& m) {
	std::string magic_number;
	int H, W, max_value;

	is >> magic_number;
	if (magic_number!="P6" != 0) return false;
	if (is.get() != '\n') return false;
	
	if (is.peek() == '#') { //commento
		uint8_t carattere = is.get();
		while (is.get() != '\n') {}
	}
	is >> W;
	if (is.get() != ' ') return false;

	is >> H;
	if (is.get() != 0x0A) return false;

	is >> max_value;
	if (is.get() != 0x0A) return false;

	m.resize(H, W);
	return true;
}


bool write_header(std::ostream& os, mat<vec3b>& m) {
	os << "P6\n";
	os << m.cols() << ' ';
	os << m.rows() << '\n' << 255 << '\n';
	return true;
}


uint8_t convert(uint8_t c) {
	if (c >= 0 and c <= 9) {
		return '0' + c;
	}
	else if (c >= 10 and c <= 35) {
		return 'a' + c - 10;
	}
	else if (c >= 36 and c <= 61) {
		return 'A' + c - 36;
	}
	else if (c == 62) return '.';
	else if (c == 63) return 0x2d;
	else if (c == 64) return ':';
	else if (c == 65) return '+';
	else if (c == 66) return '=';
	else if (c == 67) return '^';
	else if (c == 68) return '!';
	else if (c == 69) return '/';
	else if (c == 70) return '*';
	else if (c == 71) return '?';
	else if (c == 72) return '&';
	else if (c == 73) return '<';
	else if (c == 74) return '>';
	else if (c == 75) return '(';
	else if (c == 76) return ')';
	else if (c == 77) return '[';
	else if (c == 78) return ']';
	else if (c == 79) return '{';
	else if (c == 80) return '}';
	else if (c == 81) return '@';
	else if (c == 82) return '%';
	else if (c == 83) return '$';
	else if (c == 84) return '#';
	else return c;
}

uint32_t read_BE(uint32_t& caratteri) {
	uint32_t val = 0;
	val = val | ((caratteri & 0xFF000000) >> 24);
	val = val | ((caratteri & 0x00FF0000) >> 8);
	val = val | ((caratteri & 0x0000FF00) << 8);
	val = val | ((caratteri & 0x000000FF) << 24);
	return val;
}

bool encode(std::istream& is, std::ostream& os, mat<vec3b>& m, const int& N) {
	os << m.cols() << ',' << m.rows() << ',';
	//leggo 4 byte come uint32_t
	uint32_t caratteri;
	int count = 0;
	char characters[5];
	//leggo 4 byte come uint32_t
	while (raw_read(is, caratteri)) {
		//li converto in 5 caratteri 
		caratteri = read_BE(caratteri);
		for (size_t i = 0; i < 5; ++i) {
			characters[i] = caratteri / std::pow(85, 4 - i);
			caratteri -= characters[i] * std::pow(85, 4 - i);
		}
		//li codifico con la tabella e la rotazione
		for (size_t i = 0; i < 5; ++i) {
			auto x = characters[i];
			x = (x - count * N) % 85;
			if (x < 0) x += 85;
			uint8_t c = convert(x);
			os << c;
			count++;
		}
	}
	if (m.cols() * m.cols() * 3 % 4 != 0) {
		caratteri = read_BE(caratteri);
		for (size_t i = 0; i < 5; ++i) {
			characters[i] = caratteri / std::pow(85, 4 - i);
			caratteri -= characters[i] * std::pow(85, 4 - i);
		}
		//li codifico con la tabella e la rotazione
		for (size_t i = 0; i < 5; ++i) {
			auto x = characters[i];
			x = (x - count * N) % 85;
			if (x < 0) x += 85;
			uint8_t c = convert(x);
			os << c;
			count++;
		}
	}
	return true;
}

uint8_t deconvert(uint8_t c) {
	if (c >= '0' and c <= '9') {
		return c - '0';
	}
	else if (c >= 'a' and c <= 'z') {
		return c - 'a' + 10;
	}
	else if (c >= 'A' and c <= 'Z') {
		return c - 'A' + 36;
	}
	else if (c == '.') return 62;
	else if (c == 0x2d) return 63;
	else if (c == ':') return 64;
	else if (c == '+') return 65;
	else if (c == '=') return 66;
	else if (c == '^') return 67;
	else if (c == '!') return 68;
	else if (c == '/') return 69;
	else if (c == '*') return 70;
	else if (c == '?') return 71;
	else if (c == '&') return 72;
	else if (c == '<') return 73;
	else if (c == '>') return 74;
	else if (c == '(') return 75;
	else if (c == ')') return 76;
	else if (c == '[') return 77;
	else if (c == ']') return 78;
	else if (c == '{') return 79;
	else if (c == '}') return 80;
	else if (c == '@') return 81;
	else if (c == '%') return 82;
	else if (c == '$') return 83;
	else if (c == '#') return 84;
	else return c;
}

bool decode(std::istream& is, std::ostream& os, mat<vec3b>& m, int N) {
	uint8_t c1, c2;
	int H, W;
	is >> W >> c1 >> H >> c2;
	if (c1 != ',' or c2 != ',') return false;
	m.resize(H, W);
	//std::vector<uint32_t> pixels;
	char characters[5];
	int count = 0;
	write_header(os, m);
	int n_bytes = 0;
	while (raw_read(is, characters)) {
		//leggo 5 caratteri e li deconverto con tabella e rotazione 
		for (size_t i = 0; i < 5; ++i) {
			auto x = characters[i];
			uint8_t c = deconvert(x);
			c = (c + count * N) % 85;
			characters[i] = c;
			count++; // numero di byte scritti
		}
		//ricostruisco il valore in BE dei 4 byte
		uint32_t val = 0;
		n_bytes += 4;
		for (size_t i = 0; i < 5; ++i) {
			val += characters[i] * std::pow(85, 4 - i);
		}
		val = read_BE(val);
		if (n_bytes > H * W * 3) {
			os.write(reinterpret_cast<const char*>(&val), n_bytes - H * W * 3);
		}
		else {
			os.write(reinterpret_cast<const char*>(&val), 4);
		}
	}
	return true;
}
int main(int argc, char* argv[]) {
	if (argc != 5) return 1;

	std::string carattere = argv[1];
	if (carattere != "c" and carattere != "d") return 1;

	int N = std::stoi(argv[2]);
	std::ifstream is(argv[3], std::ios_base::binary);
	if (!is) return 1;
	std::ofstream os(argv[4], std::ios_base::binary);
	if (!os) return 1;
	if (carattere == "c") { //codifica
		mat<vec3b> m;
		if (read_header(is, m)) {
			if (encode(is, os, m, N)) return 0;
			else return 1;
		}
	}
	else { //decodifica
		mat<vec3b> m;
		decode(is, os, m, N);
	}
	return 0;
}