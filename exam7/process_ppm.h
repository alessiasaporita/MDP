#ifndef  PROCESS_PPM
#define PROCESS_PPM
#include "ppm.h"
#include "mat.h"
#include <string>
#include <fstream>

bool LoadPPM(const std::string& filename, mat<vec3b>& img);
void SplitRGB(const mat<vec3b>& img, mat<uint8_t>& img_r, mat<uint8_t>& img_g, mat<uint8_t>& img_b);


#endif // ! PROCESS_PPM.H
