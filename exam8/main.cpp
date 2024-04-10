#include "pcx.h"
#include "pgm.h"
#include "types.h"
#include "ppm.h"
//bool save_ppm(const std::string& filename, const mat<vec3b>& img, bool ascii = false);
int main() {
	mat<uint8_t> img;
	mat<vec3b> img2;
	load_pcx("hose.pcx", img);
	save_pgm("hose.pgm", img);
	//load_pcx("islanda_colori_24bit.pcx", img2);
	//save_ppm("islanda_colori_24bit.ppm", img2);
	//load_pcx_palette("islanda_colori_8bit.pcx", img2);
	//save_ppm("islanda_colori_8bit.ppm", img2);
	return 0;
}
