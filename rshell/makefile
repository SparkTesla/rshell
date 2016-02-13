# Compiler: Define g++ for C++
CC=g++

# compiler flags
# -Wall     : Warning Option - this turns on most, but not all, compiler warnings
# -Werror   : Make all warnings into errors
# -ansi     : In C++ mode, remove GNU extensions that conflict with ANSI C++;
#  			  This turns off certain features of GCC that are incompatible
#  			  with ANSI (when compiling C code), or of ANSI standard C++
#  			  (when compiling C++ code), such as the asm and typeof keywords...
# -pedantic : Issues all the warnings demanded by strict ANSI C and ISO C++;
#  			  reject all programs that use forbidden extensions
CFLAGS= -Wall -Werror -ansi -pedantic

# build target executable:
TARGET=rshell

all: $(TARGET)

rshell:
	mkdir bin
	$(CC) $(CFLAGS) ./src/$(TARGET).cpp -o ./bin/$(TARGET)
