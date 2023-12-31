#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <iterator>
#include <cmath>

#include "chunk.hpp"
#include "zlib.h"


//
// bmp
// 

#include <stdio.h>

//
// BMP file format header
//

const int BYTES_PER_PIXEL = 3; // red, green, & blue (3 byte => 8 bit per color)
//const int BYTES_PER_PIXEL = 6; // red, green, & blue (6 byte => 16 bit per color)
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

constexpr uint8_t TILE_HEIGHT = 8;
constexpr uint8_t TILE_WIDTH = 8;

// 1 to 256 entries, 8 bit per color (= 24 bit, RGB)
unsigned char palette[256][3];

void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);

static unsigned int png_paeth_predictor (unsigned int a, unsigned int b, unsigned int c);

static unsigned int png_unfilter_paeth (unsigned int current,
                                          unsigned int left,
                                          unsigned int above,
                                          unsigned int above_left);

void output_tile_to_bitmap(std::vector<uint8_t>* tile_vector);


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

    for (uint8_t i = 0; i < 8; i++)
    {
        uint8_t header = ifstream.get();

        // std::cout << "Header: " << header << std::endl;

        if (expectedHeader[i] != header)
        {
            return false;
        }
    }

    return true;
}

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

static z_const char hello[] = "hello, hello!";

void test_compress(Byte *compr, uLong comprLen, Byte *uncompr,
                   uLong uncomprLen) {
    int err;
    uLong len = (uLong)strlen(hello)+1;

    err = compress(compr, &comprLen, (const Bytef*)hello, len);

    printf("compress(): %s\n", (char *)compr);

    CHECK_ERR(err, "compress");

    strcpy((char*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    if (strcmp((char*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
        exit(1);
    } else {
        printf("uncompress(): %s\n", (char *)uncompr);
    }
}

// https://stackoverflow.com/questions/30079658/uncompress-error-when-using-zlib
int inf(const char *src, int srcLen, const char *dst, int dstLen) {
    z_stream strm;
    strm.zalloc = NULL;
    strm.zfree = NULL;
    strm.opaque = NULL;

    strm.avail_in = srcLen;
    strm.avail_out = dstLen;
    strm.next_in = (Bytef *) src;
    strm.next_out = (Bytef *) dst;

    int err=-1, ret=-1;
    err = inflateInit2(&strm, MAX_WBITS+16);
    if (err == Z_OK){
        err = inflate(&strm, Z_FINISH);
        if (err == Z_STREAM_END){
            ret = strm.total_out;
        }
        else{
            inflateEnd(&strm);
            return err;
        }
    }
    else{
        inflateEnd(&strm);
        return err;
    }
    inflateEnd(&strm);
    printf("%s\n", dst);
    return err;
}

void process(std::vector<char>& old_data, std::vector<char>& new_data) 
{
    printf("process()\n");
    old_data.insert(std::end(old_data), std::begin(new_data), std::end(new_data));
}

void readFile(std::ifstream& fstream, std::vector<char>& old_data, size_t size, std::function<void(std::vector<char>& old_data, std::vector<char>& new_data)> proc)
{
    printf("readFile size: %d \n", size);

    //std::ifstream f( fileName );
    std::vector<char> temp_vector(size);

    fstream.read(temp_vector.data(), temp_vector.size());
    proc(old_data, temp_vector);

    // while( fstream.read( temp_vector.data(), temp_vector.size() ) ) {
    //     temp_vector.resize( fstream.gcount() );
    //     proc( old_data, temp_vector );
    //     temp_vector.resize( size );
    // }
}

//
// CTRL + ALT + n -- Run the code
int main()
{
    /*
    Byte *uncompr;
    uLong uncomprLen = 20000;

    Byte *compr;
    uLong comprLen = 3 * uncomprLen;

    compr    = (Byte*) calloc((uInt)comprLen, 1);
    uncompr  = (Byte*) calloc((uInt)uncomprLen, 1);

    test_compress(compr, comprLen, uncompr, uncomprLen);

    free(compr);
    free(uncompr);
    */

    // std::cout << "PNG" << std::endl;




    // http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html

    // 6.1. (Sub-) Filter types
    // PNG filter method 0 defines five basic (sub-) filter types:
    //
    // Type    Name
    //
    // 0       None
    // 1       Sub
    // 2       Up
    // 3       Average
    // 4       Paeth

    // Filtering algorithms are applied to bytes, not to pixels, regardless of the bit depth or color type of the image. The filtering algorithms work on the byte sequence formed by a scanline that has been represented as described in Image layout. If the image includes an alpha channel, the alpha data is filtered in the same way as the image data.






    // http://www.schaik.com/pngsuite/

    // colour-type: 0 grayscale
    //std::string inFileName{"test_images\\Grayscale_8bits_palette_sample_image.png"};    
    
    // colour-type: 2 true color (aka. RGB)
    // has filter instructions!
    //std::string inFileName{"test_images\\DphjT.png"}; // truecolor (RGB), 8 bit,  subfilter 1 (sub), 2 (up)

    // colour-type: 2 true color (aka. RGB)
    // compression method: deflate
    // https://stackoverflow.com/questions/39019568/png-truecolor-8-bit-depth-how-to-read-idat-chunk
    //std::string inFileName{"test_images\\qOvp8.png"};
    //std::string inFileName{"test_images\\Download_Several_IDAT_Chunks.png"}; // 8 bit, truecolor (RGB), subfilter 01 (sub), 02 (up), 03 (average), 04 (paeth)
    //std::string inFileName{"test_images\\oi9n2c16.png"}; // 16 bit per color, 6 byte per pixel, filter: 4 (Paeth)
    //std::string inFileName{"test_images\\lena-005-sub-transpose.png"}; // source: https://ucnv.github.io/pnglitch/
    //std::string inFileName{"test_images\\lena-013-paeth-replace.png"}; // mixed subfilters 3 and 4

    // colour-type: 2 true color (aka. RGB) (without alpha)
    // most simple case: true color, no filter instructions!
    //std::string inFileName{"test_images\\28736.png"};
    //std::string inFileName{"test_images\\28736_2.png"}; // palette
    //std::string inFileName{"test_images\\einstein.png"};
    //std::string inFileName{"test_images\\bw_8bit_rgb_20x20.png"}; // sub-filter 1 and 2 (https://github.com/pngwriter/pngwriter/blob/master/pngs/bw_8bit_rgb_20x20.png)
    //std::string inFileName{"test_images\\burro.png"}; // 8 bit per pixel, sub-filter 4 (paeth)
    //std::string inFileName{"test_images\\28894.png"};
    //std::string inFileName{"test_images\\triforce_chamber.png"};
    //std::string inFileName{"test_images\\small_2x3.png"};
    //std::string inFileName{"test_images\\RGB_16bits_palette_sample_image.png"};
    //std::string inFileName{"test_images\\tiger-ny-png-safe-palette.png"};

    // colour-type: 3 indexed_color
    //std::string inFileName{"test_images\\plain_palette.png"}; // has two IDAT chunks !!! // 8 bit palette
    //std::string inFileName{"test_images\\zelda_alttp_overworld.png"};
    std::string inFileName{"test_images\\woods_1.png"};
    //std::string inFileName{"test_images\\witch.png"};

    // colour-type: 4 gray image with alpha channel

    // colour-type: 6 true color with alpha channel

    std::cout << "Loading file: " << std::quoted(inFileName) << std::endl;

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

    uint32_t image_width;
    uint32_t image_height;
    uint8_t colour_type;

    std::vector<char> image_data;

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

            inFile.read(reinterpret_cast<char *>(&image_width), sizeof(uint32_t));
            endswap(&image_width);
            
            inFile.read(reinterpret_cast<char *>(&image_height), sizeof(uint32_t));
            endswap(&image_height);

            std::cout << "Image Width: " << image_width << " Image Height: " << image_height << std::endl;

            uint8_t bit_depth;
            inFile.read(reinterpret_cast<char *>(&bit_depth), sizeof(uint8_t));
            std::cout << "bit_depth: " << unsigned(bit_depth) << std::endl;

            
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
        else if (strncmp(reinterpret_cast<const char *>(chunk.type), "PLTE", 4) == 0)
        {
            // push file pointer
            std::streampos current_file_position = inFile.tellg();

            std::cout << "Palette Data" << std::endl;

            // seek to chunk data
            inFile.seekg(chunk.data_offset);

            uint32_t entries = chunk.length / 3;


            for (uint32_t i = 0; i < entries; i++)
            {
                uint8_t red = 0;
                inFile.read(reinterpret_cast<char *>(&red), sizeof(uint8_t));
                uint8_t green = 0;
                inFile.read(reinterpret_cast<char *>(&green), sizeof(uint8_t));
                uint8_t blue = 0;
                inFile.read(reinterpret_cast<char *>(&blue), sizeof(uint8_t));

                palette[i][0] = red;
                palette[i][1] = green;
                palette[i][2] = blue;
            }

            // pop file pointer
            inFile.seekg(current_file_position);
        }
        else if (strncmp(reinterpret_cast<const char *>(chunk.type), "IDAT", 4) == 0)
        {
            // push file pointer
            std::streampos current_file_position = inFile.tellg();

            std::cout << "Image Data" << std::endl;

            // seek to chunk data
            inFile.seekg(chunk.data_offset);





            //image_data.reserve(chunk.length);

            readFile(inFile, image_data, chunk.length, process);

            // for (uint8_t d : image_data)
            // {
            //     printf("%02hhX ", d);
            // }

            // std::copy(std::istream_iterator<uint8_t>(inFile),
            //     std::istream_iterator<uint8_t>(),
            //     std::back_inserter(image_data));

            //inFile.read(reinterpret_cast<char *>(image_data.data()), chunk.length);
/*  
            Byte *uncompr = nullptr;
            Byte *compr = nullptr;

            // unzip
            
            uLong comprLen = chunk.length;
            uLong uncomprLen = 65536 * 1000;

            compr    = (Byte*) calloc((uInt)comprLen, 1);
            uncompr  = (Byte*) calloc((uInt)uncomprLen, 1);

            // read the chunk's data
            inFile.read(reinterpret_cast<char *>(compr), comprLen);

            // error = -4 => Z_MEM_ERROR
            // error = -3 => Z_DATA_ERROR, ???
            // error = -5 => Z_BUF_ERROR, not enough space in uncompr buffer
            //int err = uncompress(uncompr, &uncomprLen, compr, comprLen);
            //CHECK_ERR(err, "uncompress");

            // https://stackoverflow.com/questions/30079658/uncompress-error-when-using-zlib
            int err = inf((const char *)compr, comprLen, (const char *)uncompr, uncomprLen);
            CHECK_ERR(err, "uncompress");

            printf("UNCOMPRESSED\n");
*/


            //test_compress(compr, comprLen, uncompr, uncomprLen);

            // uint8_t data_byte;
            // for (uint8_t i = 0; i < chunk.length; i++)
            // {
            //     inFile.read(reinterpret_cast<char *>(&data_byte), sizeof(uint8_t));
            //     compr[i] = data_byte & 0xff;
            // }

            

            // // DEBUG output the data bytes
            // printf("COMPRESSED\n");
            // uint8_t data_byte;
            // //for (uint8_t i = 0; i < comprLen; i++)
            // for (uint8_t i = 0; i < 20; i++)
            // {
            //     printf("%02hhX ", compr[i]);
            // }
            // printf("\n");

            //int err = uncompress(uncompr, &uncomprLen, compr, comprLen);
            ///CHECK_ERR(err, "uncompress");

            //printf("UNCOMPRESSED\n");

            
/*
            // hex
            uint32_t line_width = image_width * 3 + 1;
            //for (uint32_t i = 0; i < uncomprLen; i++)
            for (uint32_t j = 0; j < image_height; j++)
            {

                uint8_t color_index = 2;
                //for (uint8_t i = 0; i < line_width; i++)
                for (uint32_t i = 0; i < line_width; i++)
                {
                    if (i != 0) {

                        // red
                        image[j][i-1][color_index] = (unsigned char) uncompr[line_width * j + i];
                        color_index--;

                        // // green
                        // image[j][i-1][color_index] = (unsigned char) uncompr[line_width * j + i];
                        // color_index--;

                        // // blue
                        // image[j][i-1][color_index] = (unsigned char) uncompr[line_width * j + i];
                        // color_index--;

                        if (color_index == -1) {
                            color_index = 2;
                        }
                    }
                    //printf("%02hhX ", uncompr[line_width * j + i]);
                }
                //printf("\n");

                //printf("\n");
                //printf("\n");
                //printf("\n");
            }
*/

/*
            int idx = 1;
            int i, j;
            for (i = 0; i < height; i++) {
                
                for (j = 0; j < width; j++) {

                    idx = (i * width + j) * 3 + (i+1);
                    
                    image[i][j][2] = (unsigned char) (unsigned char) uncompr[idx + 0]; // red
                    image[i][j][1] = (unsigned char) (unsigned char) uncompr[idx + 1]; // green
                    image[i][j][0] = (unsigned char) (unsigned char) uncompr[idx + 2]; // blue
                    
                    idx = idx + 3;
                }
            }
*/




/*
            //
            // convert image data to bitmap
            //

            int idx = 1;
            int i, j;
            for (i = 0; i < height; i++) {
                
                for (j = 0; j < width; j++) {

                    // idx is just the current pixel in the png stream but in addition,
                    // the one byte filter instruction has to be added per line of the png image data!
                    // therefore the term (i+1) is added
                    idx = (i * width + j) * 3 + (i+1);
                    
                    // height is inverted because png images are stored top-down for some reason
                    image[height-1-i][j][2] = (unsigned char) (unsigned char) uncompr[idx + 0]; // red
                    image[height-1-i][j][1] = (unsigned char) (unsigned char) uncompr[idx + 1]; // green
                    image[height-1-i][j][0] = (unsigned char) (unsigned char) uncompr[idx + 2]; // blue
                    
                    idx = idx + 3;
                }
            }

            generateBitmapImage((unsigned char*) image, height, width, imageFileName);
            printf("Image generated!!\n");
*/



/*
            //
            // convert image data to bitmap
            //

            uint32_t height = image_height;
            uint32_t width = image_width;
            //unsigned char image[height][width][BYTES_PER_PIXEL];

            uint8_t* image = new uint8_t[height * width * BYTES_PER_PIXEL];
            
            char* imageFileName = (char*) "test_images\\bitmapImage.bmp";

            // // dummy data
            // int i, j;
            // for (i = 0; i < height; i++) {
            //     for (j = 0; j < width; j++) {
            //         image[i][j][2] = (unsigned char) ( i * 255 / height );             ///red
            //         image[i][j][1] = (unsigned char) ( j * 255 / width );              ///green
            //         image[i][j][0] = (unsigned char) ( (i+j) * 255 / (height+width) ); ///blue
            //     }
            // }

            // generateBitmapImage((unsigned char*) image, height, width, imageFileName);
            // printf("Image generated!!");

            int idx = 1;
            int i, j;
            for (i = 0; i < height; i++) {
                
                for (j = 0; j < width; j++) {

                    // idx is just the current pixel in the png stream but in addition,
                    // the one byte filter instruction has to be added per line of the png image data!
                    // therefore the term (i+1) is added
                    idx = (i * width + j) * 3 + (i+1);
                    
                    // height is inverted because png images are stored top-down for some reason
                    image[ (height-1-i) * width * 3 + j * 3 + 2 ] = (unsigned char) (unsigned char) uncompr[idx + 0]; // red
                    image[ (height-1-i) * width * 3 + j * 3 + 1 ] = (unsigned char) (unsigned char) uncompr[idx + 1]; // green
                    image[ (height-1-i) * width * 3 + j * 3 + 0 ] = (unsigned char) (unsigned char) uncompr[idx + 2]; // blue
                    
                    idx = idx + 3;
                }
            }



            generateBitmapImage((unsigned char*) image, height, width, imageFileName);
            printf("Image generated!!\n");

            if (image != nullptr) {
                delete[] image;
                image = nullptr;
            }

            // // decimal
            // for (uint8_t i = 0; i < uncomprLen; i++)
            // {
            //     printf("%d ", uncompr[i]);
            // }
            // printf("\n");

            if (compr != nullptr) {
                free(compr);
                compr = nullptr;
            }
            if (uncompr != nullptr) {
                free(uncompr);
                uncompr = nullptr;
            }
*/

            // pop file pointer
            inFile.seekg(current_file_position);
        }
    }


    // uint8_t new_line = 9;

    // for (uint8_t d : image_data)
    // {
    //     printf("%02hhX ", d);
    //     new_line++;

    //     if (new_line == 16)
    //     {
    //         new_line = 0;
    //         printf("\n");
    //     }
    // }



    //
    // uncompress
    //

    Byte *uncompr = nullptr;
    Byte *compr = nullptr;

    // unzip
    
    uLong comprLen = image_data.size();
    uLong uncomprLen = 65536 * 1000;

    //compr    = (Byte*) calloc((uInt)comprLen, 1);
    uncompr  = (Byte*) calloc((uInt)uncomprLen, 1);

    // read the chunk's data
    //inFile.read(reinterpret_cast<char *>(compr), comprLen);

    // error = -4 => Z_MEM_ERROR
    // error = -3 => Z_DATA_ERROR, ???
    // error = -5 => Z_BUF_ERROR, not enough space in uncompr buffer
    //int err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    int err = uncompress(uncompr, &uncomprLen, (Byte *)image_data.data(), comprLen);
    CHECK_ERR(err, "uncompress");


/*
    //
    // output uncompressed data (colour type 3 - palette)
    //

    uint32_t new_line = 1 + image_width;
    uint32_t idx2 = 0;
    for (uint32_t i = 0; i < uncomprLen; i++)
    {
        if (idx2 == 0) {
            printf("subfilter: %d\n", (uint8_t) uncompr[i]);
        }
        printf("%02hhX ", (uint8_t) uncompr[i]);

        idx2++;
        if (idx2 >= new_line)
        {
            printf("\n");
            printf("\n");
            idx2 = 0;
        }
    }
*/

/*
    //
    // output uncompressed data (colour type 2 - true color, non-palette)
    //

    uint32_t new_line = 1 + image_width * BYTES_PER_PIXEL;
    uint32_t idx2 = 0;
    for (uint32_t i = 0; i < uncomprLen; i++)
    {
        printf("%02hhX ", (uint8_t) uncompr[i]);

        idx2++;
        if (idx2 >= new_line)
        {
            printf("\n");
            printf("\n");
            idx2 = 0;
        }
    }
*/


    // http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html

    // 6.1. (Sub-) Filter types
    // PNG filter method 0 defines five basic (sub-) filter types:
    //
    // Type    Name
    //
    // 0       None
    // 1       Sub
    // 2       Up
    // 3       Average
    // 4       Paeth

    // Filtering algorithms are applied to bytes, not to pixels, regardless of the bit depth or color type of the image. The filtering algorithms work on the byte sequence formed by a scanline that has been represented as described in Image layout. If the image includes an alpha channel, the alpha data is filtered in the same way as the image data.

    // https://glitch.art/png


    uint32_t height = image_height;
    uint32_t width = image_width;

    uint8_t* image = new uint8_t[height * width * BYTES_PER_PIXEL];

    // palette
    if (colour_type == 3) {

        int idx = 1;
        int i, j, k;
        uint8_t subfilter = 0;
        for (i = 0; i < height; i++) {
            
            for (j = 0; j < width; j++) {

                if (j == 0) {
                    subfilter = uncompr[(i * width * 1) + i];
                    printf("subfilter: %d\n", subfilter);
                }

                // idx is just the current pixel in the png stream but in addition,
                // the one byte filter instruction has to be added per line of the png image data!
                // therefore the term (i+1) is added
                idx = (i * width + j) * 1 + (i+1);

                uint8_t index_into_palette = (unsigned char) uncompr[idx];
                
                // height is inverted because png images are stored top-down for some reason
                image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 2 ] = (unsigned char) palette[index_into_palette][0]; // red
                image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 1 ] = (unsigned char) palette[index_into_palette][1]; // green
                image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 0 ] = (unsigned char) palette[index_into_palette][2]; // blue

                // next index
                idx = idx + 1;
            }
        }

    } else {


/**/
    //
    // decode image data to bitmap - filter 0 (NONE) (no action is applied at all. See: https://glitch.art/png)
    //

    

    int idx = 1;
    int i, j, k;
    uint8_t subfilter = 0;
    for (i = 0; i < height; i++) {
        
        for (j = 0; j < width; j++) {

            if (j == 0) {
                subfilter = uncompr[(i * width * BYTES_PER_PIXEL) + i];
                //printf("subfilter: %d\n", subfilter);
            }

            // idx is just the current pixel in the png stream but in addition,
            // the one byte filter instruction has to be added per line of the png image data!
            // therefore the term (i+1) is added
            idx = (i * width + j) * BYTES_PER_PIXEL + (i+1);

            // https://www.w3.org/TR/2003/REC-PNG-20031110/#9Filters
            if (subfilter == 1) {

                if (j == 0) {
                    uncompr[idx+0] = uncompr[idx+0] + 0;
                } else {
                    uncompr[idx+0] = uncompr[idx+0] + uncompr[idx+0 - 3];
                }

                if (j == 0) {
                    uncompr[idx+1] = uncompr[idx+1] + 0;
                } else {
                    uncompr[idx+1] = uncompr[idx+1] + uncompr[idx+1 - 3];
                }
                
                if (j == 0) {
                    uncompr[idx+2] = uncompr[idx+2] + 0;
                } else {
                    uncompr[idx+2] = uncompr[idx+2] + uncompr[idx+2 - 3];
                }
                
            } else if (subfilter == 2) {

                if (i == 0) {
                    uncompr[idx+0] = uncompr[idx+0] + 0;
                } else {
                    uncompr[idx+0] = uncompr[idx+0] + uncompr[idx+0 - width*BYTES_PER_PIXEL-1];
                }

                if (i == 0) {
                    uncompr[idx+1] = uncompr[idx+1] + 0;
                } else {
                    uncompr[idx+1] = uncompr[idx+1] + uncompr[idx+1 - width*BYTES_PER_PIXEL-1];
                }
                
                if (i == 0) {
                    uncompr[idx+2] = uncompr[idx+2] + 0;
                } else {
                    uncompr[idx+2] = uncompr[idx+2] + uncompr[idx+2 - width*BYTES_PER_PIXEL-1];
                }
                
            } else if (subfilter == 3) {

                uint8_t sub_a = uncompr[idx+0 - 3];
                uint8_t up_b = uncompr[idx+0 - width*BYTES_PER_PIXEL-1];
                if (j == 0) {
                    sub_a = 0;
                }
                if (i == 0) {
                    up_b = 0;
                }
                uncompr[idx+0] = uncompr[idx+0] + ((sub_a+up_b) / 2);

                sub_a = uncompr[idx+1 - 3];
                up_b = uncompr[idx+1 - width*BYTES_PER_PIXEL-1];
                if (j == 0) {
                    sub_a = 0;
                }
                if (i == 0) {
                    up_b = 0;
                }
                uncompr[idx+1] = uncompr[idx+1] + ((sub_a+up_b) / 2);
                
                sub_a = uncompr[idx+2 - 3];
                up_b = uncompr[idx+2 - width*BYTES_PER_PIXEL-1];
                if (j == 0) {
                    sub_a = 0;
                }
                if (i == 0) {
                    up_b = 0;
                }
                uncompr[idx+2] = uncompr[idx+2] + ((sub_a+up_b) / 2);
                
            }
            
            // height is inverted because png images are stored top-down for some reason
            //for (k = BYTES_PER_PIXEL-1; k >= 0; k--)
            for (k = 0; k < BYTES_PER_PIXEL; k++)
            {
                // normal
                //image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + k ] = (unsigned char) uncompr[idx + (BYTES_PER_PIXEL - (k+1))];
                
                // top down
                image[ i * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + k ] = (unsigned char) uncompr[idx + (BYTES_PER_PIXEL - (k+1))];
            }
            idx = idx + BYTES_PER_PIXEL;
        }
    }


/*
    //
    // DEBUG: convert decoded png image data to bitmap
    //

    // invert vertically since BMP is inverted vertically
    for (uint32_t height_idx = 0; height_idx < (height / 2); height_idx++)
    {
        for (uint32_t width_idx = 0; width_idx < width; width_idx++)
        {
            uint32_t top_idx = (height_idx * width + width_idx) * BYTES_PER_PIXEL;
            uint32_t bottom_idx = ((height - height_idx - 1) * width + width_idx) * BYTES_PER_PIXEL;
            uint8_t temp = 0;
            
            temp = image[bottom_idx];
            image[bottom_idx] = image[top_idx];
            image[top_idx] = temp;

            temp = image[bottom_idx + 1];
            image[bottom_idx + 1] = image[top_idx + 1];
            image[top_idx + 1] = temp;

            temp = image[bottom_idx + 2];
            image[bottom_idx + 2] = image[top_idx + 2];
            image[top_idx + 2] = temp;
        }
    }

    char* imageFileName = (char*) "test_images\\bitmapImage.bmp";
    generateBitmapImage((unsigned char*) image, height, width, imageFileName);
    printf("Image generated!!\n");

    if (image != nullptr) {
        delete[] image;
        image = nullptr;
    }
    if (compr != nullptr) {
        free(compr);
        compr = nullptr;
    }
    if (uncompr != nullptr) {
        free(uncompr);
        uncompr = nullptr;
    }
*/



/*
    //
    // decode image data to bitmap - filter 4 (paeth) (See: https://glitch.art/png)
    //

    uint32_t height = image_height;
    uint32_t width = image_width;

    uint8_t* image = new uint8_t[height * width * BYTES_PER_PIXEL];

    int idx = 1;
    int i, j;
    for (i = 0; i < height; i++) {
        
        for (j = 0; j < width; j++) {

            // idx is just the current pixel in the png stream but in addition,
            // the one byte filter instruction has to be added per line of the png image data!
            // therefore the term (i+1) is added
            idx = (i * width + j) * BYTES_PER_PIXEL + (i+1);

            uint8_t above_idx = idx - width * BYTES_PER_PIXEL
            
            // height is inverted because png images are stored top-down for some reason
            image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 2 ] = (unsigned char) (unsigned char) uncompr[idx + 0]; // red
            image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 1 ] = (unsigned char) (unsigned char) uncompr[idx + 1]; // green
            image[ (height-1-i) * width * BYTES_PER_PIXEL + j * BYTES_PER_PIXEL + 0 ] = (unsigned char) (unsigned char) uncompr[idx + 2]; // blue
            
            idx = idx + 3;
        }
    }
*/

    }

/**/
    //
    // tile search
    //

    // init all data to zero

    uint32_t id = 0;

    uint8_t tile[8 * 8 * BYTES_PER_PIXEL];
    for (uint32_t k = 0; k < 8; k++) {
        
        for (uint32_t l = 0; l < 8; l++) {
            tile[id] = 0;
            id++;
            tile[id] = 0;
            id++;
            tile[id] = 0;
            id++;
        }
    }

/*
    //
    // Test output of the first tile
    //

    id = 0;

    for (uint32_t k = 0; k < 8; k++) {
        
        for (uint32_t l = 0; l < 8; l++) {

            uint32_t idx = 0 + (k * width + l) * BYTES_PER_PIXEL;

            printf("%d: R: %d, G: %d, B: %d, \n", idx, image[idx + 0], image[idx + 1], image[idx + 2]);

            tile[id] = image[idx + 0];
            id++;
            tile[id] = image[idx + 1];
            id++;
            tile[id] = image[idx + 2];
            id++;

        }

    }

    // invert vertically since BMP is inverted vertically
    for (uint32_t height_idx = 0; height_idx < (TILE_HEIGHT / 2); height_idx++)
    {
        for (uint32_t width_idx = 0; width_idx < TILE_WIDTH; width_idx++)
        {
            uint32_t top_idx = (height_idx * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
            uint32_t bottom_idx = ((TILE_HEIGHT - height_idx - 1) * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
            uint8_t temp = 0;
            
            temp = tile[bottom_idx];
            tile[bottom_idx] = tile[top_idx];
            tile[top_idx] = temp;

            temp = tile[bottom_idx + 1];
            tile[bottom_idx + 1] = tile[top_idx + 1];
            tile[top_idx + 1] = temp;

            temp = tile[bottom_idx + 2];
            tile[bottom_idx + 2] = tile[top_idx + 2];
            tile[top_idx + 2] = temp;
        }
    }

    char* imageFileNameTile = (char*) "test_images\\tile.bmp";
    generateBitmapImage((unsigned char*) tile, TILE_HEIGHT, TILE_WIDTH, imageFileNameTile);
*/

/**/
    // working: search tiles and insert them into tile database

    std::vector<std::vector<uint8_t> *> tiles;

    for (uint32_t i = 0; i < (height / TILE_HEIGHT); i++) {
        
        for (uint32_t j = 0; j < (width / TILE_WIDTH); j++) {

            uint32_t start = i * TILE_HEIGHT * width + j * TILE_WIDTH;
            start *= BYTES_PER_PIXEL;

            id = 0;

            std::vector<uint8_t> * tile_vector = new std::vector<uint8_t>();

            // copy one tile
            for (uint32_t k = 0; k < TILE_HEIGHT; k++) {
                
                for (uint32_t l = 0; l < TILE_WIDTH; l++) {

                    uint32_t idx = start + (k * width + l) * BYTES_PER_PIXEL;

                    //printf("%d: R: %d, G: %d, B: %d, \n", idx, image[idx + 0], image[idx + 1], image[idx + 2]);

                    tile[id] = image[idx + 0];
                    tile_vector->push_back(image[idx + 0]);
                    id++;

                    tile[id] = image[idx + 1];
                    tile_vector->push_back(image[idx + 1]);
                    id++;

                    tile[id] = image[idx + 2];
                    tile_vector->push_back(image[idx + 2]);
                    id++;
                }
            }

            //
            // search current tile in list of all tiles
            //

            bool found = false;
            uint32_t tile_idx = 0;
            for (std::vector<uint8_t> * temp_tile_vector : tiles)
            {
                found = true;
                for (uint32_t tile_i = 0; tile_i < TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL; tile_i++)
                {
                    if (temp_tile_vector->at(tile_i) != tile_vector->at(tile_i)) 
                    {
                        found = false;
                        break;
                    }
                }

                if (found) {
                    //std::cout << "Tile (" << i << ", " << j << ") matches tile with index: " << tile_idx << std::endl;

                    // +1 because tiled does not count from 0 but from 1
                    std::cout << (tile_idx + 1) << ",";

                    break;
                }

                tile_idx++;
            }

            if (!found) {
                tiles.push_back(tile_vector);

                //std::cout << "Tile (" << i << ", " << j << ") is new!" << std::endl;
                //std::cout << (tiles.size() - 1) << ",";
                std::cout << (tiles.size()) << ",";
            }


            // //
            // // output tile to bmp
            // //

            // // invert vertically since BMP is inverted vertically
            // for (uint32_t height_idx = 0; height_idx < (TILE_HEIGHT / 2); height_idx++)
            // {
            //     for (uint32_t width_idx = 0; width_idx < TILE_WIDTH; width_idx++)
            //     {
            //         // find top index and corresponding bottom index
            //         uint32_t top_idx = (height_idx * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
            //         uint32_t bottom_idx = ((TILE_HEIGHT - height_idx - 1) * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
                    
            //         // swap
            //         uint8_t temp = tile[bottom_idx];
            //         tile[bottom_idx] = tile[top_idx];
            //         tile[top_idx] = temp;

            //         temp = tile[bottom_idx + 1];
            //         tile[bottom_idx + 1] = tile[top_idx + 1];
            //         tile[top_idx + 1] = temp;

            //         temp = tile[bottom_idx + 2];
            //         tile[bottom_idx + 2] = tile[top_idx + 2];
            //         tile[top_idx + 2] = temp;
            //     }
            // }

            // std::ostringstream imageFileNameTile;
            // imageFileNameTile << "test_images\\tile_" << i << "_" << j << ".bmp";

            // generateBitmapImage((unsigned char*) tile, TILE_HEIGHT, TILE_WIDTH, imageFileNameTile.str().c_str());

            // return 0;

        }

        std::cout << std::endl;
    }

    //
    // Combine all tiles into a large image
    //

    

    uint32_t tiles_per_line = 10;
    uint32_t combined_width = tiles_per_line * TILE_WIDTH;
    float number_of_lines_as_float = (float) tiles.size() / ((float) tiles_per_line);
    uint32_t number_of_lines = std::ceil(number_of_lines_as_float);

    uint32_t scan_lines = number_of_lines * TILE_HEIGHT;

    std::cout << "Total tiles: " << tiles.size() << std::endl;
    std::cout << "Tiles per line: " << tiles_per_line << std::endl;
    std::cout << "Number of lines: " << number_of_lines << std::endl;

    //int byte_size = tiles.size() * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL;
    int byte_size = number_of_lines * tiles_per_line * TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL;
    uint8_t* combined = new uint8_t[byte_size];
    for (uint32_t l = 0; l < byte_size; l++)
    {
        combined[l] = 0;
    }

    bool done = false;
    //bool done = true;

    // over all scan lines
    for (uint32_t scan_line = 0; scan_line < scan_lines; scan_line++)
    {
        // // DEBUG only the first line of tiles
        // if (scan_line == TILE_HEIGHT)
        // {
        //     done = true;
        // }

        if (done) {
            break;
        }

        // overall line of tiles
        uint32_t tile_line = scan_line / TILE_HEIGHT;

        // line inside tile
        uint32_t tile_line_mod = scan_line % TILE_HEIGHT;

        // over all tiles in a line of the combined tile map
        for (uint32_t wwidth = 0; wwidth < tiles_per_line; wwidth++)
        {
            // compute tile index in 'tiles' vector
            uint32_t tile_idx = tile_line * tiles_per_line + wwidth;

            // if (tile_idx == 304)
            // {
            //     std::cout << "tile_line: " << tile_line << " tiles_per_line: " << tiles_per_line << " wwidth: " << wwidth << std::endl;
            //     printf("304\n");
            // }

            // only access tiles that actually exist in the list of tiles
            if (tile_idx < tiles.size())
            {
                std::vector<uint8_t>* tile_ptr = tiles.at(tile_idx);

                //output_tile_to_bitmap(tile_ptr);

                // copy the current tile
                for (uint32_t ii = 0; ii < TILE_WIDTH; ii++)
                {
                    uint32_t inner_idx = tile_line_mod * TILE_WIDTH + ii;
                    inner_idx *= 3;

                    uint32_t outer_idx = scan_line * tiles_per_line * TILE_WIDTH + wwidth * TILE_WIDTH + ii;
                    outer_idx *= 3;

                    combined[outer_idx + 0] = tile_ptr->at(inner_idx + 0);
                    combined[outer_idx + 1] = tile_ptr->at(inner_idx + 1);
                    combined[outer_idx + 2] = tile_ptr->at(inner_idx + 2);
                }
            }

            //done = true;
            //break;

        }
    }

    //
    // output all tiles to combined bmp
    //

    // invert vertically since BMP is inverted vertically
    for (uint32_t height_idx = 0; height_idx < (scan_lines / 2); height_idx++)
    {
        for (uint32_t width_idx = 0; width_idx < combined_width; width_idx++)
        {
            // find top index and corresponding bottom index
            uint32_t top_idx = (height_idx * combined_width + width_idx) * BYTES_PER_PIXEL;
            uint32_t bottom_idx = ((scan_lines - height_idx - 1) * combined_width + width_idx) * BYTES_PER_PIXEL;
            
            // swap
            uint8_t temp = combined[bottom_idx];
            combined[bottom_idx] = combined[top_idx];
            combined[top_idx] = temp;

            temp = combined[bottom_idx + 1];
            combined[bottom_idx + 1] = combined[top_idx + 1];
            combined[top_idx + 1] = temp;

            temp = combined[bottom_idx + 2];
            combined[bottom_idx + 2] = combined[top_idx + 2];
            combined[top_idx + 2] = temp;
        }
    }

    std::ostringstream imageFileNameCombined;
    imageFileNameCombined << "test_images\\combined.bmp";

    generateBitmapImage((unsigned char*) combined, scan_lines, TILE_WIDTH * tiles_per_line, imageFileNameCombined.str().c_str());

    //
    // delete tile data
    // 

    for (std::vector<uint8_t> * temp_tile_vector : tiles)
    {
        delete temp_tile_vector;
    }

    return 0;
}

 // output tile to bmp
void output_tile_to_bitmap(std::vector<uint8_t>* tile_vector)
{
    uint8_t tile[TILE_WIDTH * TILE_HEIGHT * BYTES_PER_PIXEL];

    // invert vertically since BMP is inverted vertically
    for (uint32_t height_idx = 0; height_idx < TILE_HEIGHT; height_idx++)
    {
        for (uint32_t width_idx = 0; width_idx < TILE_WIDTH; width_idx++)
        {
            // find top index and corresponding bottom index
            uint32_t top_idx = (height_idx * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
            uint32_t bottom_idx = ((TILE_HEIGHT - height_idx - 1) * TILE_WIDTH + width_idx) * BYTES_PER_PIXEL;
            
            tile[bottom_idx + 0] = tile_vector->at(top_idx + 0);
            tile[bottom_idx + 1] = tile_vector->at(top_idx + 1);
            tile[bottom_idx + 2] = tile_vector->at(top_idx + 2);
        }
    }

    std::ostringstream imageFileNameTile;
    imageFileNameTile << "test_images\\tile_lul.bmp";

    generateBitmapImage((unsigned char*) tile, TILE_HEIGHT, TILE_WIDTH, imageFileNameTile.str().c_str());
}




/**
 * From: https://dox.ipxe.org/png_8c_source.html
 * 
  * Paeth predictor function (defined in RFC 2083)
  *
  * @v a                 Pixel A
  * @v b                 Pixel B
  * @v c                 Pixel C
  * @ret predictor       Predictor pixel
  */
 static unsigned int png_paeth_predictor ( unsigned int a, unsigned int b,
                                           unsigned int c ) {
         unsigned int p;
         unsigned int pa;
         unsigned int pb;
         unsigned int pc;
 
         /* Algorithm as defined in RFC 2083 section 6.6 */
         p = ( a + b - c );
         pa = abs ( p - a );
         pb = abs ( p - b );
         pc = abs ( p - c );
         if ( ( pa <= pb ) && ( pa <= pc ) ) {
                 return a;
         } else if ( pb <= pc ) {
                 return b;
         } else {
                 return c;
         }
 }

 /**
  * From: https://dox.ipxe.org/png_8c_source.html
  * 
  * Unfilter byte using the "Paeth" filter
  *
  * @v current           Filtered (raw) current byte
  * @v above_left        Unfiltered above-left byte
  * @v above             Unfiltered above byte
  * @v left              Unfiltered left byte
  * @ret current         Unfiltered current byte
  */
 static unsigned int png_unfilter_paeth ( unsigned int current,
                                          unsigned int left,
                                          unsigned int above,
                                          unsigned int above_left ) {
 
         return ( current + png_paeth_predictor ( left, above, above_left ) );
 }

 //https://dox.ipxe.org/png_8c_source.html



// uint8_t PaethPredictor (uin8_t a, uin8_t b, uin8_t c)
// {
//         // // a = left, b = above, c = upper left
//         // uint8_t p = a + b - c;        // initial estimate
//         // pa = abs(p - a);      // distances to a, b, c
//         // pb = abs(p - b);
//         // pc = abs(p - c);

//         // // return nearest of a,b,c,
//         // // breaking ties in order a,b,c.
//         // if (pa <= pb && pa <= pc) 
//         // {
//         //     return a;
//         // }
//         // else if (pb <= pc)
//         // {
//         //     return b;
//         // }
//         // else 
//         // {
//         //     return c;
//         // }

//         unsigned int p;
//          unsigned int pa;
//          unsigned int pb;
//          unsigned int pc;
 
//          /* Algorithm as defined in RFC 2083 section 6.6 */
//          p = ( a + b - c );
//          pa = abs ( p - a );
//          pb = abs ( p - b );
//          pc = abs ( p - c );
//          if ( ( pa <= pb ) && ( pa <= pc ) ) {
//                  return a;
//          } else if ( pb <= pc ) {
//                  return b;
//          } else {
//                  return c;
//          }
// }




/*
int main ()
{
    int height = 361;
    int width = 867;
    unsigned char image[height][width][BYTES_PER_PIXEL];
    char* imageFileName = (char*) "bitmapImage.bmp";

    int i, j;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            image[i][j][2] = (unsigned char) ( i * 255 / height );             ///red
            image[i][j][1] = (unsigned char) ( j * 255 / width );              ///green
            image[i][j][0] = (unsigned char) ( (i+j) * 255 / (height+width) ); ///blue
        }
    }

    generateBitmapImage((unsigned char*) image, height, width, imageFileName);
    printf("Image generated!!");
}*/

// https://devblogs.microsoft.com/oldnewthing/20210525-00/?p=105250
void generateBitmapImage(unsigned char* image, int height, int width, const char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

    int i;
    for (i = 0; i < height; i++) {
        fwrite(image + (i*widthInBytes), BYTES_PER_PIXEL, width, imageFile);
        fwrite(padding, 1, paddingSize, imageFile);
    }

    fclose(imageFile);
}

unsigned char* createBitmapFileHeader(int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}
