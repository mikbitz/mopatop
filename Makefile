#!/bin/bash
#If using MPI and the MUI coupler use the command "make WITH_MPI_COUPLER=1"
ifeq ($(WITH_MPI_COUPLER),1)
CC	= mpicxx
CXXFLAGS= -I/usr/lib64/mpi/gcc/mpich/include -std=c++17 -O3 -fopenmp -DCOUPLER
LDFLAGS = -L/usr/lib64/mpi/gcc/mpich/lib64 -lmpi -lmpichcxx -lgomp
#otherwise compile with openMP, but without MPI by just using "make"
else
CC	= g++
CXXFLAGS= -std=c++17 -O3 -fopenmp
LDFLAGS = -lgomp
endif

SRC_DIR := .
OBJ_DIR := .

#compile all sources in the SRC directory
SRC := $(wildcard $(SRC_DIR)/*.cpp)
#every SRC file has a corresponding OBJ file (extension .o)
OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC))

#assume one .d file for every .o file
DEP = $(OBJ:%.o=%.d)

#Link all object (.o) files into executable called agentModel
agentModel: $(OBJ)
	g++ $(LDFLAGS) -o $@ $^
#include all dependency files (extension .d) - this allows change in header files to be detected
-include $(DEP)

#compile all .cpp files into object (.o) files
#-MMD flag is needed for the DEP to be set up
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -MMD  -c -o $@ $<
#remove executable and all .o and .d files
clean:
	rm $(OBJ) $(DEP) agentModel	
