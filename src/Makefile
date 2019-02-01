CC=gcc
CXX=g++
CFLAGS=-g -O3
CXXFLAGS=$(CFLAGS) -std=c++11
LDFLAGS=-lm -lacc_timer

SRC_FILES=autoreload.cpp 

LIB_FILENAME=libautoreload.so
HEADER_FILENAME=autoreload.hpp

PREFIX=/usr/local

default: build

build: so

so: $(SRC_FILES) $(HEADER_FILENAME)
	$(CXX) -shared $(CXXFLAGS) -o $(LIB_FILENAME) -fPIC $(SRC_FILES) $(LDFLAGS)

install: so $(HEADER_FILENAME)
	sudo cp $(LIB_FILENAME) $(PREFIX)/lib64
	sudo cp $(HEADER_FILENAME) $(PREFIX)/include

clean:
	rm $(LIB_FILENAME)
