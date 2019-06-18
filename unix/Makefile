# Assumes pwd is unix. Also - does not provide dependency checks for .h.
ROOT = ..
BIN = $(ROOT)/bin
SRC = $(ROOT)/src
INC = $(ROOT)/include
TOOLS = $(ROOT)/tools

LIBNAME = pracrand
LIBFILE = lib$(LIBNAME).a
LIB = $(BIN)/lib$(LIBNAME).a

BINS = RNG_benchmark RNG_output RNG_test Test_calibration
TARGETS = $(LIB) $(addprefix $(BIN)/,$(BINS))

INCLUDES = $(INC) $(TOOLS)
LIBS = $(BIN)/libpracrand

CXX = g++
# CXXFLAGS = -Wall -Werror -g $(addprefix -I,$(INCLUDES))
# Many, many warnings. Too many to fix at the moment.
CXXFLAGS = -g $(addprefix -I,$(INCLUDES)) -O2
LDFLAGS = $(CXXFLAGS)

ifeq ($(shell uname -s),Linux)
CXXFLAGS += -std=c++11
LDFLAGS += -lpthread
endif

%.o: %.cpp
	$(CXX) -o $@ $(CXXFLAGS) -c $<

all: $(TARGETS)

SRC_OBJS = \
	math.o \
	non_uniform.o \
	platform_specifics.o \
	rand.o \
	sha2.o \
	test_batteries.o \
	tests.o \
	RNGs/arbee.o \
	RNGs/chacha.o \
	RNGs/efiix.o \
	RNGs/hc256.o \
	RNGs/isaac.o \
	RNGs/jsf.o \
	RNGs/mt19937.o \
	RNGs/salsa.o \
	RNGs/sfc.o \
	RNGs/sha2_based_pool.o \
	RNGs/trivium.o \
	RNGs/xsm.o \
	RNGs/other/fibonacci.o \
	RNGs/other/indirection.o \
	RNGs/other/mult.o \
	RNGs/other/simple.o \
	RNGs/other/transform.o

LIB_OBJS = \
	$(addprefix $(SRC)/,$(SRC_OBJS))

$(BIN)/libpracrand.a: $(LIB_OBJS)
	ar -r $@  $^
	ranlib $@

$(BIN)/RNG_benchmark: $(TOOLS)/RNG_benchmark.o $(LIB)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BIN)/RNG_output: $(TOOLS)/RNG_output.o $(LIB)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BIN)/RNG_test: $(TOOLS)/RNG_test.o $(LIB)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BIN)/Test_calibration: $(TOOLS)/Test_calibration.o $(LIB)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)
	find $(ROOT) \( -name "*~" -o -name "*.o" \) -exec rm -f {} \;

