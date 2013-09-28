CXX=g++
CXXFLAGS_DEBUG=-DDEBUG
CXXFLAGS_RELEASE=-O3
CXXFLAGS=-Wall -Werror -std=c++11 -Dmd_md5 -include src/cpp11_compliance.h
LDLIBS=-lcurl -lssl

SRCS=src/curleasy.cpp src/efulauncher.cpp src/target.cpp src/main.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

OUTDIR=bin
TARGET=efulauncher

RM=rm -f

.PHONY: debug
debug: CXXFLAGS+=$(CXXFLAGS_DEBUG)
debug: all

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTDIR)/$(TARGET) $(CXXFLAGS) $(OBJS) $(LDLIBS)

.PHONY: run
run: all
	cd $(OUTDIR) && ./$(TARGET) && cd ..

.PHONY: release
release: CXXFLAGS+=$(CXXFLAGS_RELEASE)
release: all

clean:
	$(RM) $(OBJS) $(OUTDIR)/$(TARGET)
