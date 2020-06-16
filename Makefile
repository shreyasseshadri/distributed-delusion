ifndef $CC
  CC = mpicc
endif

targets = linear-probing dht cached-lp overflowdht

all: $(targets)

%: %.c
	$(CC) -o $@ $@.c test_cases.c

clean:
	rm -rf *.o $(targets)
