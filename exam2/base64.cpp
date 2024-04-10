#include "base64.h"

void convert_group(uint8_t& val) {
	if (val >='A' && val <= 'Z') {
		val -= 65;
	}
	else if (val >= 'a' && val <= 'z') {
		val -= 71;
	}
	else if (val >= '0' && val <= '9') {
		val += 4;
	}
	else if (val == '+') val = 62;
	else if (val == '/') val = 63;
}

std::string base64_decode(const std::string& input) {

	std::string decoded;
	if (input.size() == 0) return decoded;

	uint8_t carattere;
	uint8_t dec = 0;
	uint8_t byte = 0;

	for (size_t i = 0; i < input.size(); ++i) {
		//6 bit
		carattere = input[i];
		convert_group(carattere); //converto secondo la tabella nel valore
		dec = (carattere << 2) & 0b11111100;
		//12 bit
		carattere = input[i+1];
		convert_group(carattere);
		byte = (carattere >> 4) & 0b00000011; 
		dec = dec | byte;
		decoded.push_back(dec);		//-> primo 
		dec = carattere & 0b00001111;
		dec = (dec << 4) & 0b11110000;
		//18 bit
		carattere = input[i + 2];
		if (carattere == '=') break;
		convert_group(carattere);
		byte = (carattere >> 2) & 0b00001111;
		dec = dec | byte;
		decoded.push_back(dec);		//-> secondo
		dec = carattere & 0b00000011;
		dec = (dec << 6) & 0b11000000;
		//24 bit
		carattere = input[i + 3];
		if (carattere == '=') break;
		convert_group(carattere);
		dec = dec | (carattere & 0b00111111);
		decoded.push_back(dec);		//-> terzo
		i += 3;
	}
	return decoded;
}


int main() {
	std::string input = "YWJj";
	//ciao sono andrea
	std::string output = base64_decode(input);
	return 0;
}
