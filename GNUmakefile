CXX=g++
CXXFLAGS_DEBUG=-Wall -Werror -std=c++11 -Dmd_md5 -DDEBUG
CXXFLAGS_RELEASE=-Wall -Werror -std=c++11 -Dmd_md5 -O3
CXXFLAGS=$(CXXFLAGS_DEBUG)
LDLIBS=-lcurl -lssl

SRCS=src/main.cpp src/curleasy.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

OUTDIR=bin
TARGET=efulauncher

RM=rm -f

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $(OUTDIR)/$(TARGET) $(CXXFLAGS) $(OBJS) $(LDLIBS)

.PHONY: run
run: all
	cd $(OUTDIR) && ./$(TARGET) && cd ..

.PHONY: release
release: CXXFLAGS=$(CXXFLAGS_RELEASE)
release: all

clean:
	$(RM) $(OBJS) $(OUTDIR)/$(TARGET)
