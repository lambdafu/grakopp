CXXFLAGS=-std=c++11 -Wall -g
LDFLAGS=-lboost_regex

all: grakopp

clang:
	~/external/extern/llvm/bin/clang++ -v -std=c++11 -stdlib=libc++  grakopp.cpp -o grakopp
