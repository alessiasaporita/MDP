#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdlib.h>
#include "bitreader.h"
#include "bitwriter.h"
#include "pam.h"

//stride -> quanti byte saltare per andare alla riga successiva, per noi W per img in grayscale o W*3 per RGB
struct nodo {
	int sym = 0;
	uint8_t len = 0;
	uint16_t code = 0;
	float probabilità = 0;
	nodo* left = nullptr;
	nodo* right = nullptr;

	nodo(){}
	nodo(float p, int v): probabilità(p), sym(v) {}
	nodo(int v, uint8_t l) : len(l), sym(v) {}

	void coding(uint16_t& codifica, uint32_t& l) {
		code = (codifica + 1);
		uint32_t count = l;
		while(count<this->len) {
			code = code << 1;
			count++;
		}
	}

};
	

bool compareFunction(nodo* &a, nodo* &b) {	
	return (a->probabilità > b->probabilità);
}

bool compareFunction2(nodo* &a, nodo* &b) {
	if (a->len == b->len) {
		return (a->sym < b->sym);
	}
	return (a->len < b->len);
}

void create_len(nodo*& root, int len) {
	if (root->left == nullptr && root->right == nullptr) { //foglia
		root->len = len;
		return;
	}
	nodo* l = root->left;
	l->len = len + 1;
	create_len(l, l->len);
	nodo* r = root->right;
	r->len = len + 1;
	create_len(r, r->len);
}

void estract_leaf(std::vector<nodo*> &root, nodo* &tree) {
	if (tree->left == nullptr && tree->right == nullptr) { //foglia
		root.push_back(tree);
		return;
	}
	nodo* l = tree->left;
	estract_leaf(root, l);
	nodo* r = tree->right;
	estract_leaf(root, r);
}

nodo* find_leaf(std::vector<nodo*>& tree, int& sym) {
	for (auto& x : tree) {
		if (x->sym == sym) return x;
	}
	return nullptr;
}

nodo* find_nodo(std::vector<nodo*>& table, uint32_t& cod, int len) {
	for (auto& x : table) {
		if (x->code == cod && x->len == len) return x;
	}
	return nullptr;
}

void create_coding(std::vector<nodo *> &tree) {
	uint32_t previous_len;
	uint16_t previous_code;
	for (size_t i = 0; i < tree.size(); ++i) {
		if (i == 0) {
			previous_len = tree[i]->len;
			previous_code = tree[i]->code;
			continue;
		}
		tree[i]->coding(previous_code, previous_len);
		previous_len = tree[i]->len;
		previous_code = tree[i]->code;
	}
}

void save_diff(std::string& filename, mat<int>& diff) {
	mat<uint8_t> diff_pam;
	diff_pam.H = diff.H;
	diff_pam.W = diff.W;
	diff_pam.max_val = 255;
	uint8_t e;
	for (int r = 0; r < diff.H; ++r) {
		for (int c = 0; c < diff.W; ++c) {
			e = diff.data[r * diff.W + c] / 2 + 128;
			diff_pam.data.push_back(e);
		}
	}
	std::string nome = filename.substr(0, filename.find(".pam"));
	std::ofstream os2(nome + "_diff2.pam", std::ios_base::binary);
	write_PAM(os2, diff_pam);
}

bool compress(std::istream& is, std::ostream& os, std::string filename) {
	mat<uint8_t> pam;
	if (!read_PAM(is, pam)) return false;
	bitwriter bw(os);

	//matrice di differenze
	mat<int> diff;
	diff.H = pam.H;
	diff.W = pam.W;
	diff.max_val = 255;
	int val, prec = 0;
	for (int r = 0; r < pam.H; ++r) {
		for (int c = 0; c < pam.W; ++c) {
			val = pam.data[r * pam.W + c] - prec; 
			prec = pam.data[r * pam.W + c];
			diff.data.push_back(val);
		}
		prec = pam.data[r * pam.W];
	}
	//salvo e visualizzo la matrice delle differenze 
	save_diff(filename, diff);

	//costruisco tabella Huffman con codici canonici

	//1- calcolo le probabilità
	float count[511] = { 0 };
	int index;
	for (int r = 0; r < diff.H; ++r) {
		for (int c = 0; c < diff.W; ++c) {
			index = diff.data[r * diff.W + c] + 255;
			count[index]++;
		}
	}

	for (auto& x : count) {
		x = x / (diff.H * diff.W);
	}
	//2- costruisco il vettore di simboli 
	std::vector<nodo*> tree;
	nodo *n;
	for (int i = 0; i < 511; ++i) {
		if (count[i] != 0) {
			n = new nodo(count[i], i - 255);
			tree.push_back(n);
		}
	}
	
	//3- ordino il vettore e unisco i due con probabilità più bassa --> ottengo l'albero 
	while (tree.size()>1) {
		std::sort(tree.begin(), tree.end(), compareFunction);
		nodo* l = tree.back();
		tree.pop_back();
		nodo* r = tree.back();
		tree.pop_back();
		n = new nodo(l->probabilità + r->probabilità, r->sym + l->sym);
		n->left = l;
		n->right = r;
		tree.push_back(n);
	}

	//4- costruisco la codifica canonica di Huffman
		//4a- calcolo le lunghezze per le codifiche delle foglie (simboli)
	nodo* root = tree[0];
	create_len(root, 0);

		//4b- creo il vettore delle foglie
	tree.resize(0);
	estract_leaf(tree, root);

		//4c- lo ordino per lunghezze crescenti
	std::sort(tree.begin(), tree.end(), compareFunction2);

		//4d- costruisco per ogni simbolo la codifica data la sua lunghezza
	create_coding(tree);

	//scrivo
	os.write("HUFFDIFF", 8);
	raw_write(os, pam.W, 4);
	raw_write(os, pam.H, 4);
	bw(tree.size(), 9);
	for (auto& x : tree) {
		bw(x->sym, 9);
		bw(x->len, 5);
	}
	for (int r = 0; r < diff.H; ++r) {
		for (int c = 0; c < diff.W; ++c) {
			nodo* elem_diff = find_leaf(tree, diff.data[r * diff.W + c]);
			bw(elem_diff->code, elem_diff->len);
		}
	}
	return true;
}

bool decompress(std::istream& is, std::ostream& os, std::string filename) {
	//leggo magic_nmumber
	char magic_number[9];
	const char s[] = "HUFFDIFF";

	//letto altezza e larghezza e creo la matrice residua
	int H, W;
	is.read(reinterpret_cast<char*>(&magic_number[0]), 8);
	magic_number[8] = 0;
	if (std::strcmp(magic_number, "HUFFDIFF")!=0) return false;
	is.read(reinterpret_cast<char*>(&W), 4);
	is.read(reinterpret_cast<char*>(&H), 4);
	mat<int> diff;
	diff.H = H;
	diff.W = W;
	diff.max_val = 255;
	diff.data.resize(0);

	//leggo n_elem tabella Huffman e la tabella
	bitreader br(is);
	uint32_t n_elem = br(9);
	std::vector<nodo*> table;
	int val;
	uint32_t len;
	for (size_t i = 0; i < n_elem; ++i) {
		val = br(9);
		if (val > 255) {
			val = val | 0b11111111111111111111111000000000;
		}
		len = br(5);
		nodo* n = new nodo(val, len);
		table.push_back(n);
	}

	//creo la tabella di Huffman
	create_coding(table);

	//decodifico i valori dei pixels
	uint32_t cod = 0;
	len = 0;
	uint8_t bit;
	nodo* my_nodo = nullptr;
	diff.data.resize(diff.W * diff.H);

	for (int r = 0; r < diff.H; ++r) {
		for (int c = 0; c < diff.W; ++c) {
			while (1) {
				bit = br(1);
				len++;
				cod = (cod << 1) | (bit & 1);
				my_nodo = find_nodo(table, cod, len);
				if (my_nodo != nullptr) {//trovato il valore
					break;
				}
			}
			diff.data[r * diff.W + c] = my_nodo->sym;
			len = 0;
			cod = 0;
		}
	}

	//salvo e visualizzo la matrice delle differenze 
	save_diff(filename, diff);

	//dalla pam diff ricostruisco la pam originale
	mat<uint8_t> pam;
	pam.H = diff.H;
	pam.W = diff.W;
	pam.max_val = 255;
	pam.data.resize(pam.H * pam.W);
	int prec = 0;
	for (int r = 0; r < diff.H; ++r) {
		for (int c = 0; c < diff.W; ++c) {
			pam.data[r * pam.W + c] = diff.data[r * pam.W + c] + prec; //riga sopra
			prec = pam.data[r * pam.W + c];
		}
		prec = pam.data[r * pam.W];
	}
	write_PAM(os, pam);
	return true;
}

int main(int argc, char* argv[]) {
	if (argc != 4) return 1;

	std::string carattere = argv[1];
	if (carattere != "c" && carattere != "d") return 1;

	std::ifstream is(argv[2], std::ios_base::binary);
	if (!is) return 1;

	std::ofstream os(argv[3], std::ios_base::binary);
	if (!os) return 1;

	if (carattere == "c"){// input = grayscale PAM, output = HUFFDIFF
		return compress(is, os, argv[2]);
	}
	else {// input = HUFFDIFF, output = grayscale PAM
		return decompress(is, os, argv[3]);
	}
}