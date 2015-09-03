HOME   := $(PWD)
CXX    := g++
CFLAGS := -Wall -Wextra -pedantic -O3
UNAME  := $(shell uname)
ifeq ($(UNAME), Linux)
LIBS   := --std=c++11 -lm -lOpenCL -lrt
endif
ifeq ($(UNAME), Darwin)
LIBS   := -std=c++11 -framework OpenCL
endif

all: clinfo

clinfo: main.cpp
	$(CXX) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f clinfo *~
