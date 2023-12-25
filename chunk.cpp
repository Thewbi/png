#include "chunk.hpp"

#include <cassert>

/**
 * https://stackoverflow.com/questions/2384111/png-file-format-endianness
 * Integers in PNG are big endian (network byte order). Windows is little
 * endian, so swap integer bytes around!
*/
bool read_chunk(std::ifstream &ifstream, Chunk& chunk, bool read_data)
{
    // the crc is initialized with all ones
    uint32_t crc = 0xffffffffL;

    // read chunk length which contains the length of the data part of the chunk
    ifstream.read(reinterpret_cast<char*>(&chunk.length), sizeof(uint32_t));
    endswap(&chunk.length);

    // read the chunk type (four byte character data)
    ifstream.read(reinterpret_cast<char*>(&chunk.type), sizeof(uint32_t));
    crc = update_crc(crc, chunk.type, sizeof(uint32_t));

    // if (read_data) {
    //     ifstream.seekg(chunk.length, std::ios::cur);
    // } else {
    //     ifstream.seekg(chunk.length, std::ios::cur);
    // }

    // store the location to the chunk data into the chunk
    chunk.data_offset = ifstream.tellg();

    constexpr uint32_t DATA_BUFFER_LENGTH = 65536;
    assert(chunk.length <= DATA_BUFFER_LENGTH);

    // read data for CRC calculation
    uint8_t data[DATA_BUFFER_LENGTH];
    ifstream.read(reinterpret_cast<char*>(data), chunk.length);
    crc = update_crc(crc, data, chunk.length);
    crc = crc ^ 0xffffffffL;
    endswap(&crc);

    // read chunk CRC from the file
    ifstream.read(reinterpret_cast<char*>(&chunk.crc), sizeof(uint32_t));
    //endswap(&chunk.crc);

    return crc == chunk.crc;
}