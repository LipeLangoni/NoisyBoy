CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -O3 -Wall -Ichess-library-master/include -std=c++17
LDFLAGS=-g -O3
LDLIBS=

SRCS=noisyboy.cpp
OBJS=$(SRCS:.cpp=.o)

.PHONY: all clean distclean

# Default target
all: noisyboy

# Link step
noisyboy: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compile step for .cpp files
%.o: %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

# Clean up object files
clean:
	$(RM) $(OBJS)

# Clean up everything, including the binary
distclean: clean
	$(RM) noisyboy
