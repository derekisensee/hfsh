#
# Sample Makefile for C and C++ programming examples
#
CC = gcc
CXX = g++ 
CFLAGS = -Wall -Werror
CXXFLAGS = -std=c++17 -Wall -Werror


#
# For this project...pick one file or the other (.c or .cpp)
# Just uncomment out the one you want to use (or comment out the other one)
#
CSRCS   = hfsh.c  
#CXXSRCS = hfsh.cpp


# Create the object files and executable files
OBJS_C    = $(CSRCS:.c=.o)
OBJS_CXX  = $(CXXSRCS:.cpp=.o)
OBJS      = $(OBJS_C) $(OBJS_CXX)

PROGS_C   = $(CSRCS:.c=)
PROGS_CXX = $(CXXSRCS:.cpp=)
PROGS     = $(PROGS_C) $(PROGS_CXX)


# This is the multi-executable target...
# If you type 'make' at the command line it will build
# all programs
.PHONY: all clean


all: $(PROGS)


# Make all programs from object files
#
# Make sure you pick the right compile action in this target
$(PROGS): %: %.o Makefile
#	$(CC) $< -o $@
	$(CXX) $< -o $@


# This line will convert any .c file to a .o file.
$(OBJS_C): %.o: %.c Makefile
	$(CC) $(CFLAGS) -c $<


# This line will convert any .cpp file to a .o file
$(OBJS_CXX): %.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -c $<


# Type 'make clean' up the subdirectory...
# Be Very Careful Not To Haphazzardly Make Changes!!!!!!!
clean:
	$(RM) $(PROGS) $(OBJS) 


