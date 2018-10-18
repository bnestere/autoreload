CC=gcc
CXX=g++
CFLAGS=-g -O3
LDFLAGS=-lm 

SRC_FILES=autoreload.cpp 

LIB_FILENAME=libautoreload.so
HEADER_FILENAME=autoreload.hpp

PREFIX=/usr/local

default: build

build: so

so: $(SRC_FILES) $(HEADER_FILENAME)
	$(CXX) -shared $(CFLAGS) -o $(LIB_FILENAME) -fPIC $(SRC_FILES) $(LDFLAGS)

install: so $(HEADER_FILENAME)
	sudo cp $(LIB_FILENAME) $(PREFIX)/lib64
	sudo cp $(HEADER_FILENAME) $(PREFIX)/include

clean:
	rm $(LIB_FILENAME)
