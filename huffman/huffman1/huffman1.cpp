#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>
#include <cstring>

template <typename T>
std::ostream& raw_write(std::ostream& os, const T& val, size_t size = sizeof(T)) {
	return os.write(reinterpret_cast<const char*>(&val), size);
}

template <typename T>
std::istream& raw_read(std::istream& is, T& val, size_t size = sizeof(T)) {
	return is.read(reinterpret_cast<char*>(&val), size);
}

class bitreader {
	uint8_t buffer_;
	int n_;
	std::istream& is_;

	uint32_t read_bit() {
		if (n_ == 0) {
			raw_read(is_, buffer_);
			n_ = 8;
		}
		--n_;
		return (buffer_ >> n_) & 1;
	}
public:
	bitreader(std::istream& is) : is_(is), n_(0) {}
	uint32_t read(uint8_t n) {
		uint32_t u = 0;
		while(n-->0) {
			u = (u << 1) | read_bit();
		}
		return u;
	}

	uint32_t operator()(uint8_t n) {
		return read(n);
	}

	bool fail() const{
		return is_.fail();
	}
	explicit operator bool() const{
		return !fail();
	}
};

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
	bitwriter(std::ostream& os): os_(os), n_(0) {}
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
	~bitwriter(){
		flush();
	}
};

struct tripletta {
	uint16_t val_;
	uint8_t codifica_ = 0;
	int len_ = 0;

	tripletta(uint16_t val, uint8_t codifica, int len): val_(val), len_(len), codifica_(codifica) {}
};

struct nodo{
	uint16_t val_;
	int frequenza_;
	nodo *left_;
	nodo *right_;
	uint8_t codifica_ = 0;
	int len_ = 0;

	nodo(uint16_t val, int frequenza, nodo* left, nodo* right) {
		val_ = val;
		frequenza_ = frequenza;
		left_ = left;
		right_ = right;
	}

	void insert_bit(uint8_t bit) {
		codifica_ = (codifica_ << 1) | bit;
		++len_;
	}
};

bool compareMyType(const void* a, const void* b)
{
	nodo* first = (nodo*)a;
	nodo* second = (nodo*)b;
	return first->frequenza_ > second->frequenza_;
}
bool compareMyType2(const void* a, const void* b)
{
	nodo* first = (nodo*)a;
	nodo* second = (nodo*)b;
	return first->val_ < second->val_;
}
bool compareMyType3(const void* a, const void* b)
{
	tripletta* first = (tripletta*)a;
	tripletta* second = (tripletta*)b;
	return first->len_ < second->len_;
}

//per costruire le codifiche
void esplora_tree(nodo* root, uint8_t codifica, int len, uint8_t bit) {
	if (root == nullptr) {
		return;
	}
	root->codifica_ = codifica;
	root->len_ = len;
	root->insert_bit(bit);
	esplora_tree(root->left_, root->codifica_, root->len_, 0);
	esplora_tree(root->right_, root->codifica_, root->len_, 1);
	return;
}

//per creare un vettore con foglie
void leaf(nodo* root, std::vector<nodo*>& my_leaf) {
	if (root == nullptr) {
		return;
	}
	//foglia
	if (root->left_ == nullptr && root->right_ == nullptr) {
		my_leaf.push_back(root);
		return;
	}
	leaf(root->left_, my_leaf);
	leaf(root->right_, my_leaf);
	return;
}

//per cercare una tripletta di codifica dato il valore
nodo * search(nodo* root, uint16_t val) {
	if (root == nullptr) {
		return root;
	}
	if (root->val_ == val) return root;
	nodo* n;
	n = search(root->left_, val);
	if (n != nullptr) return n;
	return search(root->right_, val);
}

//funzione per cercare una codifica nel vettore ordinato di triplette
bool search_carattere(std::vector<tripletta*>& triplette, uint8_t& buffer, int& len, std::ostream& os) {
	for (auto x : triplette) {
		if (x->len_ > len) return false; //appena ho un carattere di lunghezza maggiore fermo ricerca
		if (x->len_ == len && x->codifica_ == buffer) { //carattere trovato
			os.write(reinterpret_cast<const char*>(&x->val_), 1);
			buffer = 0;
			len = 0;
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]) {

	if (argc < 4) {
		return 1;
	}
	if (*argv[1] != 'c' and *argv[1] != 'd') {
		return 1;
	}
	
	std::string lettera = argv[1];
	//codifica
	if (lettera == "c") {
		std::ifstream is(argv[2], std::ios_base::binary);
		if (!is) {
			return 1;
		}
		std::ofstream os(argv[3], std::ios_base::binary);
		if (!os) {
			return 1;
		}

		int count[256] = { 0 };
		std::vector<uint8_t> caratteri; //vettori con i caratteri letti
		uint32_t n = 0; //numero di caratteri totale
		uint8_t c;
		while (is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
			caratteri.push_back(c);
			count[c]++;
			n++;
		}

		std::vector<nodo*> my_tree;
		for (size_t i = 0; i < 256; ++i) {
			if (count[i] != 0) {
				nodo *n = new nodo(i, count[i], nullptr, nullptr);
				my_tree.push_back(n);
			}
		}

		uint8_t dim;
		if (my_tree.size() == 256) dim = 0;
		else dim = my_tree.size();

		//costruisco l'albero binario di Haffman
		while (1) {
			if (my_tree.size() == 1) { //sono arrivata alla radice
				break;
			}
			std::sort(my_tree.begin(), my_tree.end(), compareMyType);
			nodo* first = my_tree.back();
			my_tree.pop_back();
			nodo* second = my_tree.back();
			my_tree.pop_back();
			uint16_t newval = first->val_ + second->val_;
			int frequenza = first->frequenza_ + second->frequenza_;
			nodo *n = new nodo(newval, frequenza, first, second);
			my_tree.push_back(n);
		}

		bitwriter bw(os);
		esplora_tree(my_tree[0], my_tree[0]->codifica_, -1, 0); //costruisco le codifiche
		os.write("HUFFMAN1", 8);
		os.write(reinterpret_cast<const char*>(&dim), 1);
		std::vector<nodo*> my_leaf;
		leaf(my_tree[0], my_leaf);
		std::sort(my_leaf.begin(), my_leaf.end(), compareMyType2);
		//scrivo tabella di Huffman
		for (auto x : my_leaf) {
			bw(x->val_, 8); //valore con 8 bit
			bw(x->len_, 5); //len con 5 bit
			bw(x->codifica_, x->len_); //codifica con len bit
		}
		bw(n, 32);
		//scrivo i simboli codificati
		for (uint8_t x :caratteri) {
			nodo *my_nodo = search(my_tree[0], x);
			bw(my_nodo->codifica_, my_nodo->len_); //scrivo simbolo 
		}
	}

	//decodifica
	if (lettera == "d") {
		std::ifstream is(argv[2], std::ios_base::binary);
		if (!is) {
			return 1;
		}
		std::ofstream os(argv[3]);
		if (!os) {
			return 1;
		}
		//HUFFMAN1
		char parola[9];
		is.read(parola, 8);
		parola[8] = 0;
		if (strcmp(parola, "HUFFMAN1") != 0) return 1;
		//numero di triplette
		uint16_t n_triplette = 0;
		is.read(reinterpret_cast<char*>(&n_triplette), 1);
		if (n_triplette == 0) n_triplette = 256;
		//leggo n triplette
		bitreader br(is);
		std::vector<tripletta *> triplette;
		for (size_t i = 0; i < n_triplette; ++i) {
			uint32_t sym = br(8);
			uint8_t len = br(5);
			uint8_t code = br(len);
			triplette.push_back(new tripletta(sym, code, len));
		}
		//ordino le triplette per lunghezza crescente
		std::sort(triplette.begin(), triplette.end(), compareMyType3);
		//numero di simboli codificati
		uint32_t n_simbols = br(32);
		uint8_t carattere = 0;
		int len = 0; //lunghezza del buffer
		uint8_t buffer = 0; //buffer (codifica fino ad ora letta)
		for (size_t i = 0; i < n_simbols; ++i) {
			if (!br) {
				return 1;
			}
			len = triplette[0]->len_;
			buffer = (buffer << len) | br(len); 
			while (!search_carattere(triplette, buffer, len, os)) { //finchè non trovo un carattere continuo a leggere
				if (!br) {
					return 1;
				}
				buffer = (buffer << 1) | br(1);
				len++;
				if (len > triplette[triplette.size() - 1]->len_) return 1; //carattere non presente nelle triplette --> errore 
			}
		}
	}

}