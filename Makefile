ifndef $CC
  CC = mpicc
endif

targets = linear-probing dht

all: $(targets)

%: %.c
	$(CC) -o $@ $@.c

clean:
	rm -rf *.o $(targets)
