#ifndef __CHUNK__
#define __CHUNK__

#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "crc.hpp"

class Chunk {
  public:
    uint32_t length;
    unsigned char type[4];
    unsigned long crc;
    uint32_t data_offset;
};

bool read_chunk(std::ifstream &ifstream, Chunk& chunk, bool read_data);

#endif