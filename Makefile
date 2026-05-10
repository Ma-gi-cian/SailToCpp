CC=g++
CFLAGS=
GDB=-ggdb -g
SOURCES=main.cpp Tokenizer/Tokenizer.cpp Parser/Parser.cpp
STANDARD=c++17

all : 
	$(CC) $(CFLAGS) -std=$(STANDARD)  $(SOURCES) -o lang && ./lang examples/sha256.sail hello.cpp

ast:
	$(CC) $(CFLAGS) -std=$(STANDARD) $(SOURCES) -o lang && ./lang examples/sha256.sail 

gdb : 
	$(CC) $(GDB) -std=$(STANDARD) $(SOURCES) -o debug && gdb --args ./lang examples/sha256.sail hello.cpp

clean:
	rm lang
