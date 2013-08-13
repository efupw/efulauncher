cc=g++
cflags=-Wall -Werror -std=c++11

include=-I./
ldflags=
ldlibs=-lcurl

sources=src/main.cpp
outdir=bin
outfile=efulauncher

.PHONY: all

all: build

test: cflags += -DDEBUG
test: all
	cd $(outdir) && ./$(outfile) && cd ..

build: $(sources)
	$(cc) -o $(outdir)/$(outfile) $(include) $(cflags) $(ldflags) $(ldlibs) $(sources)

$(outfile): $(patsubst %.cpp,%.o,$(sources))
