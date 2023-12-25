main: util.hpp crc.hpp crc.cpp chunk.hpp chunk.cpp main.cpp
	g++.exe -g -Wall -o main.exe crc.cpp chunk.cpp main.cpp -I. -Izlib-1.3 zlib-1.3\libz.a

.PHONY: clean

clean:
	rm -f *.o main.exe