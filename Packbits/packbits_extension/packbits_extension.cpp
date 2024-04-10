#include <stdlib.h>
#include <fstream>
#include <vector>
#include <cstring>

void stampa_copy(std::vector<uint8_t>& caratteri, std::ofstream &os) {
	uint8_t n = caratteri.size() - 1;
	os.write(reinterpret_cast<const char*>(&n), 1);
	for (auto x : caratteri) {
		os.write(reinterpret_cast<const char*>(&x), 1);
	}
}

void stampa_run(uint8_t c, std::ofstream& os, int n) {
	uint8_t n_run = 257 - n;
	os.write(reinterpret_cast<const char*>(&n_run), 1);
	os.write(reinterpret_cast<const char*>(&c), 1);
}

int main(int argc, char* argv[]) {
	if (argc != 4) return 1;
	
	std::string lettera = argv[1];
	if (lettera != "c" and lettera != "d") return 1;

	std::ifstream is(argv[2], std::ios_base::binary);
	if (!is) return 1;
	std::ofstream os(argv[3], std::ios_base::binary);
	if (!os) return 1;

	if (lettera == "c") {
		std::vector<uint8_t> caratteri;
		uint8_t c; //carattere attuale
		uint8_t tmp; //carattere attuale
		/*
		status: 0 = stato iniziale
		status: 1 = letto un carattere
		status: 2 = RUN
		status: 3 = COPY
		*/
		int status_attuale;
		int status_precedente;
		int n = 0; //numero caratteri in una run
		while (is.read(reinterpret_cast<char*>(&c), 1)) {
			if (n == 0) {
				n = 1;
				status_precedente = 0;
				status_attuale = 1;
				tmp = c;
				continue;
			}
			else {
				if (n == 128) { //run finita
					stampa_run(tmp, os, n);
					n = 1;
					tmp = c;
					status_attuale = 1;
					status_precedente = 2;
					continue;
				}
				if (caratteri.size() == 129) { //copy finita
					tmp = caratteri.back();
					caratteri.pop_back();
					stampa_copy(caratteri, os);
					caratteri.resize(0);
					n = 1;
					status_attuale = 1;
					status_precedente = 3;
				}
				if (c == tmp) {
					if (status_attuale == 2 or status_attuale == 1) {
						if (status_attuale == 1) {
							status_attuale = 2;
							status_precedente = 1;
						}
						n++;
						if (n > 2) {
							if (caratteri.size() != 0) { //stampa copy
								caratteri.pop_back();
								stampa_copy(caratteri, os);
								caratteri.resize(0);
							}
						}
						continue;
					}
					if (status_attuale == 3) {
						status_attuale = 2;
						status_precedente = 3;
						n = 2;
						continue;
					}
				}
				else { //c!=tmp
					if (status_attuale == 2) {
						if (n == 2 && status_precedente == 3) { //copy continua
							caratteri.push_back(tmp);
							caratteri.push_back(c);
							status_attuale = 3;
							tmp = c;
							continue;
						}
						stampa_run(tmp, os, n);
						status_precedente = 2;
						status_attuale = 1;
						n = 1;
						tmp = c;
						continue;
					}
					if (status_attuale == 3 or status_attuale == 1) {
						if (status_attuale == 1) {
							caratteri.push_back(tmp);
							status_attuale = 3;
							status_precedente = 1;
						}
						caratteri.push_back(c);
						tmp = c;
						continue;
					}
				}
			}
		}
		if (status_attuale == 2) {
			if (n == 2) {
				caratteri.push_back(c);
				stampa_copy(caratteri, os);
			}
			else stampa_run(c, os, n);
		}
		if (status_attuale == 3 or status_attuale == 1) {
			stampa_copy(caratteri, os);
		}
		uint8_t eod = 128;
		os.write(reinterpret_cast<const char*>(&eod), 1);
	}
	if (lettera == "d") {
		uint8_t L;
		uint8_t c;
		while (is.read(reinterpret_cast<char*>(&L), 1)) {
			if (L >= 0 && L <= 127) { //copy
				for (size_t i = 0; i < L + 1; ++i) {
					if(!is.read(reinterpret_cast<char*>(&c), 1)) return 1;
					os.write(reinterpret_cast<const char*>(&c), 1);
				}
			}
			if (L >= 129 && L <= 255) { //run
				uint8_t n_run = 257 - L;
				if (!is.read(reinterpret_cast<char*>(&c), 1)) return 1;
				for (size_t i = 0; i < n_run; ++i) {
					os.write(reinterpret_cast<const char*>(&c), 1);
				}
			}
			if (L == 128) { //eod
				break;
			}
		}
	}
}