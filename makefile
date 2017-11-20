CXX=g++
CFLAGS=-std=c++14 -DDEBUG -g -pg -DUSE_SDL
LIBS=-lSDLmain -lSDL
VPATH = src
OBJ = attotime.o cga.o cpu808x.o interface.o main.o mda.o misc.o ram.o savestate.o

all: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o superpc $(LIBS)

clean:
	rm *.o && rm superpc

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<
