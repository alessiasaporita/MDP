#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>


void print_tab(int count) {
	for (int i = 0; i < count; ++i) {
		std::cout << "	";
	}
}
struct elem {
	char id; //'i': integers, 's': string, 'l': list, 'd':dictionary
	std::string stringa;
	int integer;
	std::vector<elem> lista;
	std::map<std::string, elem> dictionary;

	elem(){}
	elem(std::istream& is) {
		char identifier = is.peek();
		switch (identifier) {
		break; case 'i': { //intero
			id = is.get();
			is >> integer;
			char c = is.get(); //'e'
		}
		break; case 'l': { //lista
			id = is.get();
			while (is.peek() != 'e') {
				lista.emplace_back(is);
			}
			char c = is.get(); //'e'
		}
		break; case 'd': { //dizionario
			id = is.get();
			elem e;
			while (is.peek() != 'e') {
				e = elem(is);//key
				dictionary[e.stringa] = elem(is);
			}
			char c = is.get(); //'e'
		}
		break; default: {//stringa
			id = 's';
			int len = 0;
			is >> len;
			if(is.get()!=':') throw; //':'
			stringa.resize(len);
			is.read(reinterpret_cast<char*>(&stringa[0]), len);
		}
		}
	}
	void print(int count = 0) {
		switch (id)
		{
		break; case 'i': {//intero
			std::cout << integer << "\n";
		}
		break; case 'l': {//lista
			std::cout << "[\n";
			for (auto& e : lista) {
				print_tab(count + 1);
				e.print(count + 1);
			}
			print_tab(count);
			std::cout << "]\n";
		}
		break; case 'd': { //dizionario
			std::cout << "{\n";
			for (auto& x : dictionary) {
				print_tab(count + 1);
				std::cout << "\"" << x.first << "\" => ";
				if (x.first == "pieces") {
					std::cout << "\n";
					print_tab(count + 2);
					for (int i = 0; i < x.second.stringa.size(); ++i) {
						if (i % 20 == 0 && i!=0) {
							std::cout << "\n";
							print_tab(count + 2);
						}
						std::cout << std::hex << std::setw(2) << std::setfill('0') << int((unsigned char)x.second.stringa[i]);
					}
					std::cout << "\n";
					std::cout << std::dec;
				}
				else {
					x.second.print(count + 1);
				}
			}
			print_tab(count);
			std::cout << "}\n";
		}
		break; default: {//stringa
			std::cout << "\"";
			for (const auto& ch : stringa) {
				if (ch > 126 || ch < 32) {
					std::cout << ".";
				}
				else {
					std::cout << ch;
				}
			}
			std::cout << "\"\n";
		}	
		}
	}
};


int main(int argc, char* argv[]) {
	if (argc != 2) {
		return -1;
	}
	std::ifstream is(argv[1], std::ios_base::binary);
	if (!is) {
		return -1;
	}
	elem root(is);
	root.print();
	return 0;
}