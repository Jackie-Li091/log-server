CC=g++
CFLAGS=-I
CFLAGS+=-Wall
#CFLAGS+=-DDEBUG
FILES=LogServer.cpp
LIBS=-lpthread

LogServer: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o LogServer
	
all: LogServer