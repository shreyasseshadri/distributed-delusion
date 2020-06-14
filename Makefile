ifndef $CC
  CC = mpicc
endif

targets = linear-probing dht cached-lp

all: $(targets)

%: %.c
	$(CC) -o $@ $@.c

clean:
	rm -rf *.o $(targets)
