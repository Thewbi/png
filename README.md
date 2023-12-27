# png

## Compiling zlib on Windows 10

1. Extract the zlib13.zip file to the folder zlib-1.3
2. cd zlib-1.3
3. make -fwin32/Makefile.gcc

Error: This occurs on 64-bit Cygwin installations:
cannot export gzopen_w: symbol not defined

Solution: 
From C:\Users\wolfg\dev\cpp\png\zlib-1.3\win32\zlib.def, delete the line gzopen_w

Follow this post: https://github.com/madler/zlib/issues/45
Replace:  ifdef _WIN32 by if defined(_WIN32) || defined(CYGWIN)

4. copy zlib1.dll into the folder that contains the .exe file that uses zlib

## Building with zlib from Gygwin

Make sure zlib is installed using the cygwin installer.

In the makefile, append:

```
-IC:\cygwin64\usr\include -LC:\cygwin64\lib -llibz
```


## Decoding PNG

PNG is made up of chunks. (Tip: use the page https://www.nayuki.io/page/png-file-chunk-inspector to inspect chunks!)

The image data is stored inside IDAT chunks. A png file can contain more than one IDAT chunk.

The image data originally is a consecutive stream of bytes. 
A encoder is free to split up the stream into as many IDAT chunks as it wants. 

The job of the decoder is to reassemble all IDAT chunks it finds into a single stream / buffer of bytes.

Once the image data is retrieved from the IDAT chunks, the image data has to be uncompressed / inflated.
The zlib library is used to uncompress the data.

Once the image data is uncompressed, the result is image data in it's filtered form.
PNG filtering belongs to the compression backend because the purpose of filtering is
to act as a preparation step to compression. The benefit of filtering is to convert
the image data into a form that leads to a increased compression performance and makes
the png file smaller.

To remove the filtering after decompression, the filter method that was used during
encoding is read from the IHDR chunk. Once the filter method is known, the decoder
has to apply the invers filtering method to get rid of the filer.

During decoding, after decompression, the applied filter has to be inverted.

The applied filter method is stored inside the IHDR chunk. 
Currently there is only a single filter method defined which is filter method 0.

Filter method 0 specifies that a subfilter is applied to each row of the image data.
For filter method 0, 5 possible subfilters are defined:

0       None (no modification to the raw data is applied!)
1       Sub
2       Up
3       Average
4       Paeth

In case the filter method is 0 (the only defined filter method yet), the decoder
has to figure out which of the subfilters has been used.

The subfilters are actually applied to each scanline of the original image data.
This means a subfilter was applied to each row of the image data.

The PNG specification for filter method 0 says that a single byte is prepended
to each row of uncompressed image data. This byte contains the subfilter that
is applied to this row of the image.

Each row can have another subfilter applied to it. So the decoder has to apply
the correct filter that is used for this row.

Therefore the decoder combines all IDAT chunks into a stream of bytes.
It processes each scanline in that stream. Each scanline has a single subfilter
byte. The subfilter byte is read and the subfilter is removed from the scanline
to retrieve the unfiltered, uncompressed image data.

Once the image data is available it has to be converted to pixel color data.
In case the image is of color type 2 (truecolor, RGB), then the image data
is the raw RGB data (8 or 16 bit) of each pixel already.

In case the image is of color type 3 (palette) then the image data is a one-
byte index into the palette. The palette then stores the raw RGB value for that
pixel. 

The palette is stored inside the PLTE chunk. It is not compressed/deflated
so there is no need to use the zlib on it!
The length of PLTE chunk has to be a multiple of 3. Each entry in the PLTE chunk 
is 3 byte long. 1 byte per color of an RGB value. A RGB value has one byte for red, 
one byte for green and one byte for blue in that order.


## Implementing a decoder

1. Open a png file and write a parser for chunks including CRC check
2. Learn how to assemble all IDAT chunks into a single stream / buffer
3. Learn how to inflate/uncompress the stream / buffer (from the IDAT chunks) using zlib
4. Take the uncompressed data and remove the filter / subfilter from each row
producing an buffer of raw data. Learn how to revert the sub, up, average, and thd paeth subfilter. 

The raw data has the sub filter removed and also has the
one leading byte removed that stores the sub-filter type.
5. Depending on the colour type, the raw data is now palettized or true color data.
True color data can be 8 or 16 bit per channel (channel = red or green or blue in RGB).
6. Learn how to resolve palettized raw data into a windows BMP image.
7. Learn how to resolve 8 bit and 16 true color into a windows BMP image.
8. Write the windows bmp to disk.