CXX=g++
CFLAGS=-std=c++14
LIBS=-lmingw32 -lSDLmain -lSDL -mwindows
VPATH = src
OBJ = attotime.o cga.o cpu.o interface.o main.o mda.o misc.o ram.o savestate.o

all: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o superpc $(LIBS)
	
debug:
	$(CXX) $(CFLAGS) -g -pg $(OBJ) -o superpc $(LIBS) -DDEBUG

clean:
	rm *.o && rm superpc

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<
