#ifndef PGM16_H
#define PGM16_H

#include <cstdint>
#include <string>
#include "mat.h"

bool load(const std::string& filename, mat<uint16_t>& img, uint16_t& maxvalue);

#endif

