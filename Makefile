SRC=$(wildcard src/**/*.cpp) src/chani.cpp
HDR=$(wildcard src/**/*.h)

chani: $(SRC) $(HDR)
	c++ -o chani -std=c++2a -O3 -I src $(SRC)
