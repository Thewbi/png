#ifndef __CHUNK__
#define __CHUNK__

#include <iostream>
#include <iomanip>
#include <fstream>

#include "util.hpp"
#include "crc.hpp"

class Chunk {
  public:
    uint32_t length; // 4 byte
    unsigned char type[4]; // 4 byte
    uint32_t crc; // 4 byte

    // file pointer value to point to the start of data
    uint32_t data_offset;
};

bool read_chunk(std::ifstream &ifstream, Chunk& chunk, bool read_data);

#endif