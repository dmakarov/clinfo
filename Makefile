HOME   := $(PWD)
CXX    := g++
CFLAGS := -std=c++11 -Wall -Wextra -pedantic -O3
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
