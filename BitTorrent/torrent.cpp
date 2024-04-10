#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdint>
#include <exception>
#include <iostream>
#include <iomanip>
//#include <format>

struct elemento {
	char tipo_; //'d' = dizionario, 'l'=lista, 's'=stringa, 'i'=intero
	std::string stringa_;
	std::map<std::string, elemento *> coppie_; //coppie chiave valore per dizionario
	std::vector<elemento *> list_values_;	//elementi della lista
	uint64_t intero_ = 0;

	elemento() {}
	elemento(char tipo) : tipo_(tipo) {}
	
	void set_stringa(std::string& s) { //se di tipo stringa
		stringa_.resize(s.size());
		for (size_t i = 0; i < s.size(); ++i) {
			stringa_[i] = s[i];
		}
	}
};

elemento* create_tree(char character, std::istream& is) {
	char c;
	if (character == 'd') { //dictionary
		elemento* d = new elemento(character);
		while (1) {
			//key
			c = is.get();
			if (c == 'e') return d; //fine dizionario
			elemento* elem = create_tree(c, is);
			//value
			c = is.get();
			elemento* elem2 = create_tree(c, is);
			(*d).coppie_.insert({ (*elem).stringa_, elem2 });
		}
	}
	else if (character == 'l') { //lista
		elemento* l = new elemento(character);
		while (1) {
			c = is.get();
			if (c == 'e') return l; //fine lista
			elemento* elem = create_tree(c, is);
			(*l).list_values_.push_back(elem);
		}
	}
	else if (character == 'i') { //intero
		elemento* i = new elemento(character);
		uint64_t integer;
		is >> integer;
		(*i).intero_ = integer;
		c = is.get();
		if (c != 'e') throw;
		return i;
	}
	else { //numero ---> stringa, character == lunghezza
		std::string stringa;
		stringa.push_back(character);
		while (1) {
			c = is.get();
			if (c == ':') break;
			stringa.push_back(c);
		}
		int length = std::stoi(stringa);
		stringa.resize(0);
		for (size_t i = 0; i < length; ++i) {
			stringa.push_back(is.get());
		}
		elemento* elem = new elemento('s');
		(*elem).set_stringa(stringa);
		return elem;
	}
}

void print_tab(const int& n) {
	char tab = '	';
	for (size_t i = 0; i < n; ++i) std::cout << tab;
}

void print_elemento(elemento* e, int& count) {
	if (e->tipo_ == 'd') { //dizionario
		for (const auto& couple : e->coppie_) {
			print_tab(count);
			std::cout << "\"" << couple.first << "\"" << " => ";
			if (couple.first == "pieces") { //caso particolare
				count++;
				for (size_t c = 0; c < couple.second->stringa_.size(); c++) {
					if (c % 20 == 0) {
						std::cout << "\n";
						print_tab(count);
					}
					//std::cout << std::format("{:02x}", unsigned char(couple.second->stringa_[c]));
					std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)(unsigned char)couple.second->stringa_[c];
				}
				std::cout << "\n";
				count--;
				continue;
			}
			if (couple.second->tipo_ == 'l') { //lista dentro dizionario
				std::cout << "[\n";
				count++;
				print_elemento(couple.second, count);
				count--;
				print_tab(count);
				std::cout << "]\n";
				continue;
			}
			else if (couple.second->tipo_ == 'd') { //dizionario dentro dizionario
				std::cout << "{\n";
				count++;
				print_elemento(couple.second, count);
				count--;
				print_tab(count);
				std::cout << "}\n";
			}
			else { //stringa o intero dentro dizionario
				int c = 0; 
				print_elemento(couple.second, c);
			}
		}
	}
	else if (e->tipo_ == 'i') {//intero
		print_tab(count);
		std::cout << e->intero_ << '\n';
	}
	else if (e->tipo_ == 'l') {//lista
		for (size_t i = 0; i < e->list_values_.size(); ++i) {
			if (e->list_values_[i]->tipo_ == 'l') { //lista dentro lista
				print_tab(count);
				std::cout << "[\n";
				count++;
				print_elemento(e->list_values_[i], count);
				count--;
				print_tab(count);
				std::cout << "]\n";
				continue;
			}
			else if (e->list_values_[i]->tipo_ == 'd') { //dizionario dentro lista
				print_tab(count);
				std::cout << "{\n";
				count++;
				print_elemento(e->list_values_[i], count);
				count--;
				print_tab(count);
				std::cout << "}\n";
				continue;
			}
			else { //stringa o intero dentro lista
				print_elemento(e->list_values_[i], count);
				continue;
			}
		}
	}
	else if (e->tipo_ == 's') {//stringa
		print_tab(count);
		for (size_t j = 0; j < e->stringa_.size(); ++j) {
			if (e->stringa_[j] < 32 || e->stringa_[j] > 126) {
				e->stringa_[j] = '.';
			}
		}
		std::cout << "\"" << e->stringa_ << "\"\n";
	}
}


int main(int argc, char* argv[]) {
	if (argc != 2) return -1;

	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) return -1;
	char c;
	c = is.get();
	if (c != 'd') return -1;
	elemento* e = create_tree(c, is);
	int count = 1;
	std::cout << "{\n";
	print_elemento(e, count);
	std::cout << "}\n";
	return 0;
}