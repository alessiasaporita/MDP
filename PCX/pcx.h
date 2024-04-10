#ifndef PCX_H
#define PCX_H

#include "mat.h"
#include <string>
#include <array>

using vec3b = std::array<uint8_t, 3>;
bool load_pcx(const std::string& filename, mat<uint8_t>& img);
bool load_pcx(const std::string& filename, mat<vec3b>& img);

#endif // !PCX_H

