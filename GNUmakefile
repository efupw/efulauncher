CXXFLAGS_DEBUG=-DDEBUG
CXXFLAGS_RELEASE=-O3
CXXFLAGS=-Wall -Werror -std=c++11 -Dmd_md5 -include src/cpp11_compliance.h
LDLIBS=-lcurl -lssl -lcrypto

MODULES=curleasy efulauncher target main
MODE=debug
SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=symbols
OBJS=$(addprefix $(OBJ_DIR)/$(MODE)/,$(addsuffix .o,$(MODULES)))
TARGET=$(BIN_DIR)/$(MODE)/efulauncher

RM=rm -f

.PHONY: debug
debug:
	$(MAKE) --no-print-directory CXXFLAGS="$(CXXFLAGS) $(CXXFLAGS_DEBUG)" \
	    MODE=$(@) $(BIN_DIR)/$(@)/efulauncher

.PHONY: release
release:
	$(MAKE) --no-print-directory CXXFLAGS="$(CXXFLAGS) $(CXXFLAGS_RELEASE)" \
	    MODE=$(@) $(BIN_DIR)/$(@)/efulauncher

.PHONY: all
all:
	$(MAKE) --no-print-directory debug
	$(MAKE) --no-print-directory release

$(TARGET): $(OBJS)
	$(shell test -d $(@D) || mkdir -p $(@D))
	$(CXX) -o $(@) $(OBJS) $(LDLIBS)

$(OBJ_DIR)/$(MODE)/%.o: $(SRC_DIR)/%.cpp
	$(shell test -d $(@D) || mkdir -p $(@D))
	$(CXX) -c -o $(@) $(CXXFLAGS) $(LDLIBS) $(<)

.PHONY: run
run:
	cd $(BIN_DIR)/$(MODE) && ./efulauncher && cd -

.PHONY: clean
clean:
	$(RM) -r symbols

.PHONY: clobber
clobber: clean
	$(RM) -r $(BIN_DIR)
