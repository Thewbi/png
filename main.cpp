#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>

#include "chunk.hpp"

/**
 * // https://www.w3.org/TR/2003/REC-PNG-20031110/#5DataRep
 */
bool checkHeader(std::ifstream &ifstream)
{
    unsigned int expectedHeader[8];
    expectedHeader[0] = 137;
    expectedHeader[1] = 80;
    expectedHeader[2] = 78;
    expectedHeader[3] = 71;
    expectedHeader[4] = 13;
    expectedHeader[5] = 10;
    expectedHeader[6] = 26;
    expectedHeader[7] = 10;

    for (int i = 0; i < 8; i++)
    {
        unsigned int header = ifstream.get();

        // std::cout << "Header: " << header << std::endl;

        if (expectedHeader[i] != header)
        {
            return false;
        }
    }

    return true;
}

//
// CTRL + ALT + n -- Run the code
int main()
{
    // std::cout << "PNG" << std::endl;

    std::string inFileName{"C:\\Users\\U5353\\Downloads\\zelda_alttp_overworld_tiles.png"};

    std::cout << "Loading file " << std::quoted(inFileName) << std::endl;

    std::ifstream inFile{inFileName, std::ios::binary};
    if (!inFile)
    {
        std::cerr << "File " << std::quoted(inFileName) << " did not open" << std::endl;

        return 1;
    }

    if (!checkHeader(inFile))
    {
        std::cerr << "File " << std::quoted(inFileName) << " does not have a valid PNG header!" << std::endl;
        return 2;
    }

    // This signature indicates that the remainder of the datastream contains
    // a single PNG image, consisting of a series of chunks beginning with an
    // IHDR chunk and ending with an IEND chunk.

    while (inFile.peek() != EOF)
    {

        Chunk chunk;

        bool read_data = false;
        if (!read_chunk(inFile, chunk, read_data))
        {
            std::cerr << "File " << std::quoted(inFileName) << " could not load chunk!" << std::endl;
            return 3;
        }

        std::cout << chunk.type[0] << chunk.type[1] << chunk.type[2] << chunk.type[3] << std::endl;

        if (strncmp(reinterpret_cast<const char *>(chunk.type), "IHDR", 4) == 0)
        {

            int current_file_position = inFile.tellg();

            // https://www.w3.org/TR/2003/REC-PNG-20031110/#11IHDR
            std::cout << "Image Header" << std::endl;

            inFile.seekg(chunk.data_offset);

            uint32_t width;
            inFile.read(reinterpret_cast<char *>(&width), sizeof(uint32_t));
            endswap(&width);

            uint32_t height;
            inFile.read(reinterpret_cast<char *>(&height), sizeof(uint32_t));
            endswap(&height);

            std::cout << "Width: " << width << " Height: " << height << std::endl;

            uint8_t bit_depth;
            inFile.read(reinterpret_cast<char *>(&bit_depth), sizeof(uint8_t));
            std::cout << "bit_depth: " << unsigned(bit_depth) << std::endl;

            uint8_t colour_type;
            inFile.read(reinterpret_cast<char *>(&colour_type), sizeof(uint8_t));
            std::cout << "colour_type: " << unsigned(colour_type);

            switch (colour_type)
            {
            case 0:
                std::cout << " (Greyscale)";
                break;

            case 2:
                std::cout << " (Truecolour)";
                break;

            case 3:
                std::cout << " (Indexed-colour)";
                break;

            case 4:
                std::cout << " (Greyscale with alpha)";
                break;

            case 6:
                std::cout << " (Truecolour with alpha)";
                break;

            default:
                std::cout << " (unknown)";
                break;
            }

            std::cout << std::endl;

            uint8_t compression_method;
            inFile.read(reinterpret_cast<char *>(&compression_method), sizeof(uint8_t));
            std::cout << "compression_method: " << unsigned(compression_method) << std::endl;

            uint8_t filter_method;
            inFile.read(reinterpret_cast<char *>(&filter_method), sizeof(uint8_t));
            std::cout << "filter_method: " << unsigned(filter_method) << std::endl;

            uint8_t interlace_method;
            inFile.read(reinterpret_cast<char *>(&interlace_method), sizeof(uint8_t));
            std::cout << "interlace_method: " << unsigned(interlace_method) << std::endl;

            inFile.seekg(current_file_position);
        }
    }

    return 0;
}
