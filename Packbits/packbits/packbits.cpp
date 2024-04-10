#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>


void stampa_copy(std::vector<uint8_t>& caratteri, std::ostream &os) {
	//stampo la copy
	uint8_t n_copy = caratteri.size() - 1;
	os.write(reinterpret_cast<const char*>(&n_copy), 1);
	for (auto x : caratteri) { //stampo il buffer di caratteri
		os.write(reinterpret_cast<const char*>(&x), 1);
	}
}

void stampa_run(int n, uint8_t tmp, std::ostream& os) {
	uint8_t n_run = 257 - n;
	os.write(reinterpret_cast<const char*>(&n_run), 1);
	os.write(reinterpret_cast<const char*>(&tmp), 1);
}

int main(int argc, char* argv[]) {
	if (argc != 4) return 1;

	std::string lettera = argv[1];
	if (lettera != "c" && lettera != "d") return 1;

	std::ifstream is(argv[2], std::ios_base::binary);
	if (!is) return 1;

	std::ofstream os(argv[3], std::ios_base::binary);
	if (!os) return 1;

	if (lettera == "c") {
		std::vector<uint8_t> caratteri;
		uint8_t c; //carattere attuale
		uint8_t tmp; //carattere precedente
		int n = 0; //numero di caratteri in una run
		/*
		status: 0 = stato iniziale
		status: 1 = letto un carattere
		status: 2 = RUN
		status: 3 = COPY		
		*/
		int status_attuale = 0; 
		while (is.read(reinterpret_cast<char*>(&c), 1)) {
			//lettura primo carattere
			if (n == 0) {
				n = 1;
				status_attuale = 1;
				tmp = c;
				continue;
			}
			else {//ho letto almeno un carattere
				if (n == 128) {//RUN finisce
					stampa_run(n, tmp, os);
					tmp = c;
					n = 1;
					status_attuale = 1;
					continue;
				}
				if (caratteri.size() == 129) { //copy finita
					tmp = caratteri.back();
					caratteri.pop_back();
					stampa_copy(caratteri, os);
					caratteri.resize(0);
					n = 1;
					status_attuale = 1;
				}
				if (tmp == c) {
					if (status_attuale == 2 || status_attuale==1) { //RUN or 1CARATTERE
						n++;
						if (status_attuale == 1) status_attuale = 2;
						continue;
					}
					if (status_attuale == 3) { //COPY
						caratteri.pop_back(); //estraggo ultimo carattere letto
						stampa_copy(caratteri, os);
						caratteri.resize(0);
						n = 2; 
						status_attuale = 2; //RUN
						continue;
					}
				}
				else { //c!=tmp
					if (status_attuale == 2) { //RUN
						stampa_run(n, tmp, os);
						tmp = c;
						n = 1;
						status_attuale = 1;
						continue;
					}
					if (status_attuale == 3 || status_attuale == 1) {
						if (status_attuale == 1) {
							status_attuale = 3;
							caratteri.push_back(tmp);
						}
						caratteri.push_back(c);
						tmp = c;
						continue;
					}
				}
			}
			
		}
		if (status_attuale == 2) { //RUN
			stampa_run(n, tmp, os);
		}
		if (status_attuale == 3 || status_attuale == 1) { //copy
			stampa_copy(caratteri, os);
		}
		uint8_t eod = 128;
		os.write(reinterpret_cast<const char*>(&eod), 1);
	}

	if (lettera == "d") {
		std::vector<uint8_t> caratteri;
		uint8_t L;
		uint8_t c;
		while (is.read(reinterpret_cast<char*>(&L), 1)) {
			if (L == 128) break;
			if (L <= 127 && L >=0) { //COPY
				for (size_t i = 0; i < L + 1; ++i) { //leggo i prossimi L+1 caratteri
					if(!is.read(reinterpret_cast<char*>(&c), 1)) return 1;
					caratteri.push_back(c);
				}
				for (auto x : caratteri) { //scrivo i caratteri letti su file
					os.write(reinterpret_cast<const char*>(&x), 1);
				}
				caratteri.resize(0);
			}
			if (L <= 255 && L >= 129) { //RUN
				uint8_t n = 257 - L;
				if (!is.read(reinterpret_cast<char*>(&c), 1)) return 1;
				for (size_t i = 0; i < n; ++i) {
					os.write(reinterpret_cast<const char*>(&c), 1);
				}
			}
		}
	}
}