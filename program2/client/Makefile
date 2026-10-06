MAKE = make
CC = gcc
CFLAGS = -O2 -Wall
OBJS = main.o client.o
EXE = client

all: $(EXE)
.c.o:	
	$(CC) -c $< $(CFLAGS)
$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

clean:
	rm -f $(EXE) $(OBJS) *~ *.o *.core
