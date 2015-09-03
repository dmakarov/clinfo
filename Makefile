HOME   := $(PWD)
CXX    := g++
CFLAGS := -Wall -Wextra -pedantic -O3 -std=c++11
UNAME  := $(shell uname)
ifeq ($(UNAME), Linux)
LIBS   := -lm -lOpenCL -lrt
endif
ifeq ($(UNAME), Darwin)
LIBS   := -framework OpenCL
endif

all: clinfo

clinfo: main.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f clinfo *~
