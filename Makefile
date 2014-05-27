CC=g++
CFLAGS=-std=c++11
LIBS=-lSDL

all:
	$(CC) $(CFLAGS) src/*.cpp -o superpc $(LIBS)
	
debug:
	$(CC) $(CFLAGS) -g -pg src/*.cpp -o superpc $(LIBS) -DDEBUG

clean:
	rm superpc
