ifndef $CC
  CC = mpicc
endif

CFLAGS = -O3

SOURCE = dht.c

EXE = dht

all: $(EXE)

$(EXE): $(SOURCE)
	$(CC) $(CFLAGS) -o $(EXE) $(SOURCE)

clean:
	rm -rf *~ *.o dht
