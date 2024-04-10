#include <stdlib.h>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>


struct nodo {
	int len_ = -1;
	uint32_t freq_;
	uint32_t sym_;
	uint16_t codifica_ = 0;
	nodo* left_;
	nodo* right_;

	nodo(uint32_t freq, uint32_t sym, nodo* left, nodo *right): freq_(freq), sym_(sym), left_(left), right_(right){}
};

struct coppia {
	int len_ = -1;
	uint32_t sym_;
	uint16_t codifica_ = 0;
	coppia(uint32_t sym, int len): len_(len), sym_(sym){}
};

class bitreader {
	uint8_t buffer_;
	std::ifstream& is_;
	int n_;

	uint32_t read_bit() {
		if (n_ == 0) {
			is_.read(reinterpret_cast<char*>(&buffer_), sizeof(buffer_));
			n_ = 8;
		}
		--n_;
		return  (buffer_ >> n_) & 1;
	}

public:
	bitreader(std::ifstream& is): is_(is), n_(0){}

	uint32_t read(uint8_t n) {
		uint32_t u = 0;
		while (n-- > 0) {
			u = (u << 1) | read_bit();
		}
		return u;
	}

	uint32_t operator ()(uint8_t n) {
		return read(n);
	}

	bool fail() const {
		return is_.fail();
	}
	explicit operator bool() const {
		return !fail();
	}

};

class bitwriter {
	uint8_t buffer_;
	std::ostream& os_;
	int n_;

	std::ostream& write_bit(uint8_t bit) {
		buffer_ = (buffer_ << 1) | (bit & 1);
		++n_;
		if (n_ == 8) {
			os_.write(reinterpret_cast<const char*>(&buffer_), sizeof(buffer_));
			n_ = 0;
		}
		return os_;
	}
public:
	bitwriter(std::ostream& os) : n_(0), os_(os) {}

	std::ostream& write(uint32_t u, uint8_t n) {
		for (int i = n - 1; i >= 0; --i) {
			write_bit(u >> i);
		}
		return os_;
	}

	std::ostream& operator()(uint32_t u, uint8_t n) {
		write(u, n);
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

template<typename T>
void crea_codifica(std::vector<T> &my_leaf) {
	for (size_t i = 0; i < my_leaf.size(); ++i) {
		if (i == 0) continue;
		my_leaf[i]->codifica_ = my_leaf[i - 1]->codifica_ + 1;
		if (my_leaf[i]->len_ > my_leaf[i - 1]->len_) {
			my_leaf[i]->codifica_ = my_leaf[i]->codifica_ << (my_leaf[i]->len_ - my_leaf[i - 1]->len_);
		}
	}
}

void calcola_len(nodo *root, int len) {
	if (root == nullptr) return;
	root->len_ = len + 1;
	calcola_len(root->left_, root->len_);
	calcola_len(root->right_, root->len_);
	return;
}

void estrai_leaft(nodo* root, std::vector<nodo*> &my_leaft) {
	if (root->left_ == nullptr && root->right_ == nullptr) {
		my_leaft.push_back(root);
		return;
	}
	estrai_leaft(root->left_, my_leaft);
	estrai_leaft(root->right_, my_leaft);
	return;
}

bool compare(void* a, void* b) {
	nodo *first = (nodo*)a;
	nodo *second = (nodo*)b;
	return first->freq_ > second->freq_;
}
bool compare3(void* a, void* b) {
	nodo* first = (nodo*)a;
	nodo* second = (nodo*)b;
	if (first->len_ == second->len_) return first->sym_ < second->sym_;
	else return first->len_ < second->len_;
}


nodo* search(std::vector<nodo*>& my_leaf, uint8_t x) {
	//lo cerco nel vettore di codifica
	for (size_t i = 0; i < my_leaf.size(); ++i) {
		if (x == my_leaf[i]->sym_) {
			return my_leaf[i];
		}
	}
	return nullptr;
}

//cerca se c'è una codifica uguale a buffer 
bool cerca_codifica(std::vector<coppia *> &coppie, uint8_t &buffer, int len, std::ofstream &os) {
	for (auto x : coppie) {
		if (x->len_ > len) return false;
		if (x->len_ == len & x->codifica_ == buffer) { //simbolo trovato
			os.write(reinterpret_cast<const char*>(&x->sym_), 1);
			buffer = 0;
			len = 0;
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {

	if (argc != 4) {
		return 1;
	}

	std::string lettera = argv[1];
	if (lettera == "c") {
		std::ifstream is(argv[2], std::ios_base::binary);
		if (!is) {
			return 1;
		}
		std::ofstream os(argv[3], std::ios_base::binary);
		if (!os) {
			return 1;
		}
		uint8_t count[256] = { 0 };
		std::vector<uint8_t> caratteri;
		uint8_t c;
		while (is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
			++count[c];
			caratteri.push_back(c);
		}
		std::vector<nodo*> my_tree;
		for (size_t i = 0; i < 256; ++i) {
			if (count[i] != 0) {
				nodo* n = new nodo(count[i], i, nullptr, nullptr);
				my_tree.push_back(n);
			}
		}
		std::sort(my_tree.begin(), my_tree.end(), compare);
		//costruisco albero
		while(1) {
			if (my_tree.size() == 1) break;
			std::sort(my_tree.begin(), my_tree.end(), compare);
			nodo* first = my_tree.back();
			my_tree.pop_back();
			nodo* second = my_tree.back();
			my_tree.pop_back();
			nodo* new_nodo = new nodo(first->freq_ + second->freq_, first->sym_ + second->sym_, first, second);
			my_tree.push_back(new_nodo);
		}
		//calcolo lunghezze
		calcola_len(my_tree[0], my_tree[0]->len_);
		//estraggo le foglie e le ordino per lunghezze e caratteri 
		std::vector<nodo*> my_leaf;
		estrai_leaft(my_tree[0], my_leaf);
		std::sort(my_leaf.begin(), my_leaf.end(), compare3);
		//costruisco codifica
		crea_codifica(my_leaf);
		os.write("HUFFMAN2", 8);
		uint16_t n_car= my_leaf.size();
		if (n_car == 256) n_car = 0;
		os.write(reinterpret_cast<const char*>(&n_car), 1);
		bitwriter bw(os);
		//scrivo le coppie di codifica
		for (size_t i = 0; i < my_leaf.size(); ++i) {
			bw(my_leaf[i]->sym_, 8);
			bw(my_leaf[i]->len_, 5);
		}
		bw(caratteri.size(), 32);
		//scrivo il testo cifrato
		for (auto x: caratteri) {
			nodo *my_nodo = search(my_leaf, x);
			bw(my_nodo->codifica_, my_nodo->len_);
		}
	}
	if (lettera == "d") {
		std::ifstream is(argv[2], std::ios_base::binary);
		if (!is) {
			return 1;
		}
		std::ofstream os(argv[3], std::ios_base::binary);
		if (!os) {
			return 1;
		}
		char par[9];
		is.read(reinterpret_cast<char*>(par), 8);
		par[8] = 0;
		if (strcmp(par, "HUFFMAN2") != 0) return 1;
		uint16_t n_codifiche = 0; //numero di coppie di Huffman
		is.read(reinterpret_cast<char*>(&n_codifiche), 1);
		if (n_codifiche == 0) n_codifiche = 256;
		//leggo le coppie
		bitreader br(is);
		std::vector<coppia *> coppie;
		for (size_t i = 0; i < n_codifiche; ++i) {
			uint8_t sym = br(8);
			int len = br(5);
			coppie.push_back(new coppia(sym, len));
		}
		uint32_t n_simboli = br(32);
		crea_codifica(coppie);
		//leggo i simboli codificati
		uint8_t buffer = 0;
		int len; //numero di bit nel buffer
		for (size_t i = 0; i < n_simboli; ++i) {
			if (!br) {
				return 1;
			}
			len = coppie[0]->len_;
			buffer = (buffer << len) | br(len);
			while (!cerca_codifica(coppie, buffer, len, os)) {
				if (!br) {
					return 1;
				}
				buffer = (buffer << 1) | br(1);
				++len;
				if (len > coppie[coppie.size() - 1]->len_) return 1; //errore nel file
			}
		}
	}
}