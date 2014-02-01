HOME    = $(PWD)
CC      = gcc
CPP	= g++
CFLAGS  = -Wall -Wextra -pedantic -O3

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
INCL_P  = -I$(HOME)/inc -I/usr/local/cuda/include
LIBS   = -lm -lOpenCL -lrt
INCL_AMD = -I$(HOME)/inc -I$(AMDAPPSDKROOT)/include 
LIBS_AMD = -L$(AMDAPPSDKROOT)/lib/x86_64 $(LIBS)
CFLAGS_AMD  = $(CFLAGS) -DATI_OS_LINUX 
endif
ifeq ($(UNAME), Darwin)
INCL_P   = -I$(HOME)/inc
LIBS     = -framework OpenCL
INCL_AMD = $(INCL_P) 
LIBS_AMD = $(LIBS)
CFLAGS_AMD = $(CFLAGS)
endif

all: clinfo

clinfo: clinfo.c
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@rm -f clinfo *~
