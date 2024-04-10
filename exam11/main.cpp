#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>
#include <exception>
#include <map>
#include <cstring>

using namespace std;

#include "ppm.h"
#include "image_operations.h"

struct elem {
	uint8_t marker = 0; //'A' = array, 'O' = object
	bool b = 0;
	int8_t i = 0;
	uint8_t U = 0;
	int16_t I = 0;
	int32_t l = 0;
	float d = 0;
	double D = 0;
	char C = 0;
	std::string S;
	std::vector<elem> array_elem;
	std::map<std::string, elem> obj;
	char tipo_array;

	int32_t as_int() const {
		switch (marker) {
		break; case 'i': return i;
		break; case 'I': return I;
		break; case 'l': return l;
		break; default: throw;
		}
	}

	elem() {}

	elem(std::istream& is) : elem(is, is.get()) {}

	elem(std::istream& is, uint8_t type) {
		marker = type;
		switch (marker) {
		break; case 'Z':
		break; case 'N':
		break; case 'T': {
			b = true;
		}
		break; case 'F': {
			b = false;
		}
		break; case 'i': {
			is.read(reinterpret_cast<char*>(&i), 1);
		}
		break; case 'U': {
			is.read(reinterpret_cast<char*>(&U), 1);
		}
		break; case 'I': {
			I = is.get();
			I = (I << 8) | is.get();
		}
		break; case 'l': {
			l = is.get();
			l = (l << 8) | is.get();
			l = (l << 8) | is.get();
			l = (l << 8) | is.get();
		}
		break; case 'd': {
			char buf[4];
			is.read(buf, sizeof buf);
			std::reverse_copy(begin(buf), end(buf), reinterpret_cast<char*>(&d));
		}
		break; case 'D': {
			char buf[8];
			is.read(buf, sizeof buf);
			std::reverse_copy(begin(buf), end(buf), reinterpret_cast<char*>(&D));
		}
		break; case 'C': {
			is.read(reinterpret_cast<char*>(&C), sizeof(C));
		}
		break; case 'S': {
			auto len = elem(is).as_int();
			S.resize(len);
			is.read(reinterpret_cast<char*>(&S[0]), len);
		}
		break; case '{': { //oggetto
			marker = 'O';
			int i = 0;
			std::string key;
			while (is.peek() != '}') {
				auto len = elem(is).as_int();
				key.resize(len);
				is.read(reinterpret_cast<char*>(&key[0]), len);
				if (obj.find(key) != obj.end()) {
					key += to_string(i);
					++i;
				}
				obj[key] = elem(is);
			}
			is.get();
		}
		break; case '[': { //array
			marker = 'A';
			//optimization
			if (is.peek() == '$') {
				is.get();
				tipo_array = is.get(); //'U'
				if (is.get() != '#') break;
				int32_t n_elem = elem(is).as_int();
				for (int32_t i = 0; i < n_elem; ++i) {
					array_elem.emplace_back(is, tipo_array);
				}
			}
			else {
				while (is.peek() != ']') {
					array_elem.emplace_back(is);
				}
				is.get();
			}
		}
		break; default: throw;
		}
	}
};

int convert(const string& sInput, const string& sOutput)
{
	std::ifstream is(sInput, std::ios_base::binary);
	if (!is) {
		return 1;
	}

	elem root(is);
	uint8_t marker = root.obj["canvas"].obj["width"].marker;
	int h = 0, w = 0;
	switch (marker)
	{
	break; case 'i':  w = root.obj["canvas"].obj["width"].i;
	break; case 'l':  w = root.obj["canvas"].obj["width"].l;
	break; case 'U':  w = root.obj["canvas"].obj["width"].U;
	break; case 'I':  w = root.obj["canvas"].obj["width"].I;
	default: break;
	}
	marker = root.obj["canvas"].obj["height"].marker;
	switch (marker)
	{
	break; case 'i':  h = root.obj["canvas"].obj["height"].i;
	break; case 'I':  h = root.obj["canvas"].obj["height"].I;
	break; case 'l':  h = root.obj["canvas"].obj["height"].l;
	break; case 'U':  h = root.obj["canvas"].obj["height"].U;
	default: break;
	}
	vec3b sfondo;
	sfondo[0] = root.obj["canvas"].obj["background"].array_elem[0].U;
	sfondo[1] = root.obj["canvas"].obj["background"].array_elem[1].U;
	sfondo[2] = root.obj["canvas"].obj["background"].array_elem[2].U;
	image<vec3b> img(w, h);
	for (int r = 0; r < img.height(); ++r) {
		for (int c = 0; c < img.width(); ++c) {
			img(c, r) = sfondo;
		}
	}
	if (!writeP6("canvas.ppm", img)) return EXIT_FAILURE;


	// Dal file UBJ devo estrarre le informazioni sulle immagini da incollare su img
	vec3b pixel;
	int i = 0;
	int x = 0, y = 0;
	for (const auto& primitiva : root.obj["elements"].obj) {
		std::string name = primitiva.first;
		if(isdigit(name[name.size()-1]))
			name.resize(primitiva.first.size() - 1);
		if (name == "image") {
			auto object = primitiva.second.obj;
			//width
			marker = object["width"].marker;
			switch (marker)
			{
			break; case 'i':  w = object["width"].i;
			break; case 'I':  w = object["width"].I;
			break; case 'l':  w = object["width"].l;
			break; case 'U':  w = object["width"].U;
			default: break;
			}
			//height
			marker = object["height"].marker;
			switch (marker)
			{
			break; case 'i':  h = object["height"].i;
			break; case 'I':  h = object["height"].I;
			break; case 'l':  h = object["height"].l;
			break; case 'U':  h = object["height"].U;
			default: break;
			}
			//x
			marker = object["x"].marker;
			switch (marker)
			{
			break; case 'i':  x = object["x"].i;
			break; case 'I':  x = object["x"].I;
			break; case 'l':  x = object["x"].l;
			break; case 'U':  x = object["x"].U;
			default: break;
			}
			//y
			marker = object["y"].marker;
			switch (marker)
			{
			break; case 'i':  y = object["y"].i;
			break; case 'I':  y = object["y"].I;
			break; case 'l':  y = object["y"].l;
			break; case 'U':  y = object["y"].U;
			default: break;
			}			
			image<vec3b> image(w, h);
			int count = 0;
			for (int r = 0; r < image.height(); ++r) {
				for (int c = 0; c < image.width(); ++c) {
					pixel[0] = object["data"].array_elem[count].U;
					pixel[1] = object["data"].array_elem[count+1].U;
					pixel[2] = object["data"].array_elem[count+2].U;
					count += 3;
					image(c, r) = pixel;
				}
			}
			paste(img, image, x, y);
			std::string index = std::to_string(i+1);
			writeP6("image" + index + ".ppm", image);
		}
		else {
			continue;
		}
	}

	// Output in formato PPM
	if (!writeP6(sOutput, img)) return EXIT_FAILURE;
	
	//stampa
	for (const auto& primitiva : root.obj["elements"].obj) {
		std::string name = primitiva.first;
		if (isdigit(name[name.size() - 1]))
			name.resize(primitiva.first.size() - 1);
		std::cout << name << " : ";
		for (const auto& attr : primitiva.second.obj) {
			std::cout << attr.first << ',';
		}
		std::cout << '\n';
	}
	
	return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		return 1;
	}

	string sInput = argv[1];
	string sOutput = argv[2];
	return convert(sInput, sOutput);
}
