#ifndef PCX_H
#define PCX_H

#include <string>
#include "mat.h"
#include "types.h"

//bool load_pcx(const std::string& filename, mat<uint8_t>& img);
//bool load_pcx(const std::string& filename, mat<vec3b>& img);
bool load_pcx(const std::string& filename, mat<vec3b>& img);

#endif // !PCX_H

