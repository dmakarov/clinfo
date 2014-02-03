HOME   := $(PWD)
CC     := gcc
CXX    := g++
CFLAGS := -Wall -Wextra -pedantic -O3
UNAME  := $(shell uname)
ifeq ($(UNAME), Linux)
LIBS   := -lm -lOpenCL -lrt
endif
ifeq ($(UNAME), Darwin)
LIBS   := -framework OpenCL
endif

all: clinfo

clinfo: clinfo.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f clinfo *~
