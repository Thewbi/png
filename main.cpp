#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>

#include "chunk.hpp"
#include "zlib.h"


//
// bmp
// 

#include <stdio.h>

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);





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

//
// CTRL + ALT + n -- Run the code
int main()
{
    Byte *uncompr;
    uLong uncomprLen = 20000;

    Byte *compr;
    uLong comprLen = 3 * uncomprLen;

    compr    = (Byte*) calloc((uInt)comprLen, 1);
    uncompr  = (Byte*) calloc((uInt)uncomprLen, 1);

    test_compress(compr, comprLen, uncompr, uncomprLen);

    free(compr);
    free(uncompr);

    // std::cout << "PNG" << std::endl;

    //std::string inFileName{"test_images\\zelda_alttp_overworld.png"}; // colour-type: indexed_color
    //std::string inFileName{"test_images\\28736.png"};
    //std::string inFileName{"test_images\\28894.png"};
    //std::string inFileName{"test_images\\DphjT.png"};
    std::string inFileName{"test_images\\triforce_chamber.png"};
    //std::string inFileName{"test_images\\small_2x3.png"};

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

    uint32_t image_width;
    uint32_t image_height;

    while (inFile.peek() != EOF)
    {
        Chunk chunk;

        bool read_data = false;
        //read_chunk(inFile, chunk, read_data);
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
        else if (strncmp(reinterpret_cast<const char *>(chunk.type), "IDAT", 4) == 0)
        {



            int height = image_height;
            int width = image_width;
            unsigned char image[height][width][BYTES_PER_PIXEL];
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




            // push file pointer
            int current_file_position = inFile.tellg();

            std::cout << "Image Data" << std::endl;

            // seek to chunk data
            inFile.seekg(chunk.data_offset);

/**/
            // unzip
            Byte *uncompr;
            uLong uncomprLen = 65536*100;

            Byte *compr;
            uLong comprLen = chunk.length;

            compr    = (Byte*) calloc((uInt)comprLen, 1);
            uncompr  = (Byte*) calloc((uInt)uncomprLen, 1);

            //test_compress(compr, comprLen, uncompr, uncomprLen);

            // uint8_t data_byte;
            // for (uint8_t i = 0; i < chunk.length; i++)
            // {
            //     inFile.read(reinterpret_cast<char *>(&data_byte), sizeof(uint8_t));
            //     compr[i] = data_byte & 0xff;
            // }

            inFile.read(reinterpret_cast<char *>(compr), comprLen);

            // // DEBUG output the data bytes
            // printf("COMPRESSED\n");
            // uint8_t data_byte;
            // //for (uint8_t i = 0; i < comprLen; i++)
            // for (uint8_t i = 0; i < 20; i++)
            // {
            //     printf("%02hhX ", compr[i]);
            // }
            // printf("\n");

            int err = uncompress(uncompr, &uncomprLen, compr, comprLen);
            CHECK_ERR(err, "uncompress");

            printf("UNCOMPRESSED\n");

            
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

            // // dec
            // for (uint8_t i = 0; i < uncomprLen; i++)
            // {
            //     printf("%d ", uncompr[i]);
            // }
            // printf("\n");

            free(compr);
            free(uncompr);


            // pop file pointer
            inFile.seekg(current_file_position);
        }
    }

    return 0;
}









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


void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
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

unsigned char* createBitmapFileHeader (int height, int stride)
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
