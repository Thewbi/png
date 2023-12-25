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