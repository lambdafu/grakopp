CXXFLAGS=-std=c++11 -Wall -g -Iinclude -O4
LDFLAGS=-lboost_regex

all: grakopp

grakopp: include/grakopp/grakopp.hpp grakopp.cpp

clang:
	~/external/extern/llvm/bin/clang++ -I. -std=c++11 -stdlib=libc++  grakopp.cpp -o grakopp
