CC=g++
CFLAGS=-Wall -Wextra
SOURCES=main.cpp Tokenizer/Tokenizer.cpp
STANDARD=c++17

all : 
	$(CC) $(CFLAGS) -std=$(STANDARD)  $(SOURCES) -o lang && ./lang examples/sha256.sail

clean:
	rm main
