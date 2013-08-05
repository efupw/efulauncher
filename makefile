cc=g++
cflags=-Wall -Werror -std=c++11

include=-I/usr/local/include
include=
ldflags=-L/usr/local/lib
ldflags=
ldlibs=-lcurl

sources=src/main.cpp
out=bin/efulauncher

.PHONY: all

all: build

build: $(sources)
	$(cc) -o $(out) $(include) $(cflags) $(ldflags) $(ldlibs) $(sources)

$(out): $(patsubst %.cpp,%.o,$(sources))
