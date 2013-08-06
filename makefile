cc=g++
cflags=-Wall -Werror -std=c++11

include=-I/usr/local/include
include=
ldflags=-L/usr/local/lib
ldflags=
ldlibs=-lcurl

sources=src/main.cpp
outdir=bin
outfile=efulauncher

.PHONY: all

all: build

test: all
	cd $(outdir) && ./$(outfile) && cd ..

build: $(sources)
	$(cc) -o $(outdir)/$(outfile) $(include) $(cflags) $(ldflags) $(ldlibs) $(sources)

$(outfile): $(patsubst %.cpp,%.o,$(sources))
