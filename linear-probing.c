#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define SUCCESS 0
#define FAILURE 1

#define UNOCCUPIED 0
#define OCCUPIED 1

struct Pair
{
	int meta;
	int key;
	int value;
};

struct DHT
{
	struct Pair *pairs;
	int n_process;
	int ht_length;
	int ht_length_per_process;
	MPI_Win store_win;
};

int hash(struct DHT *dht, int key)
{
	return key % dht->ht_length;
}

int get(struct DHT *dht, int key, int *value)
{
	struct Pair local_pair;
	int hash_key = hash(dht, key);
	int destRank = hash_key / dht->ht_length_per_process;
	int destPos = hash_key - dht->ht_length_per_process * destRank;
	int iniRank = destRank, iniPos = destPos;
	while (1)
	{
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
		MPI_Get(&local_pair, sizeof(local_pair), MPI_BYTE,
				destRank, destPos,
				sizeof(local_pair), MPI_BYTE, dht->store_win);
		MPI_Win_unlock(destRank, dht->store_win);

		if (local_pair.meta == OCCUPIED)
		{
			if (local_pair.key == key)
			{
				*value = local_pair.value;
				return SUCCESS;
			}
			if (++destPos >= dht->ht_length_per_process)
			{
				destPos = 0;
				destRank++;
				if (destRank >= dht->n_process)
					destRank = 0;
			}
			if (destRank == iniRank && destPos == iniPos)
			{
				return FAILURE;
			}
		}
		else
		{
			return FAILURE;
		}
	}
}

int insert(struct DHT *dht, int key, int value)
{
	struct Pair local_pair;
	struct Pair pair = {meta : OCCUPIED, key : key, value : value};
	int hash_key = hash(dht, key);
	int destRank = hash_key / dht->ht_length_per_process;
	int destPos = hash_key - dht->ht_length_per_process * destRank;
	int iniRank = destRank, iniPos = destPos;
	while (1)
	{
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
		MPI_Get(&local_pair, sizeof(local_pair), MPI_BYTE,
				destRank, destPos,
				sizeof(local_pair), MPI_BYTE, dht->store_win);
		MPI_Win_unlock(destRank, dht->store_win);

		if (local_pair.meta == UNOCCUPIED)
		{
			MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
			MPI_Put(&pair, sizeof(pair), MPI_BYTE,
					destRank, destPos,
					sizeof(pair), MPI_BYTE, dht->store_win);
			MPI_Win_unlock(destRank, dht->store_win);
			return SUCCESS;
		}
		if (++destPos >= dht->ht_length_per_process)
		{
			destPos = 0;
			destRank++;
			if (destRank >= dht->n_process)
				destRank = 0;
		}
		if (destRank == iniRank && destPos == iniPos)
		{
			return FAILURE;
		}
	}
	return FAILURE;
}

int init(struct DHT *dht, MPI_Info info, MPI_Comm comm, int table_size)
{
	int size;
	MPI_Comm_size(comm, &size);
	assert(table_size % size == 0);

	dht->n_process = size;
	dht->ht_length = table_size;
	dht->ht_length_per_process = table_size / size;
	dht->pairs = calloc(dht->ht_length_per_process, sizeof(struct Pair));
	memset(dht->pairs, UNOCCUPIED, sizeof(struct Pair) * dht->ht_length_per_process);

	MPI_Win_create(dht->pairs, dht->ht_length_per_process * sizeof(struct Pair),
				   sizeof(struct Pair), info, comm, &dht->store_win);
}

int deinit(struct DHT *dht)
{
	free(dht->pairs);
}

void test_dht(struct DHT *dht, int n, int *keys, int *values)
{
	for (int i = 0; i < n; i++)
	{
		assert(insert(dht, keys[i], values[i]) == SUCCESS);
	}
	int value;
	for (int i = 0; i < n; i++)
	{
		assert(get(dht, keys[i], &value) == SUCCESS && value == values[i]);
	}
}

int main(int argc, char **argv)
{
	struct DHT dht;
	int N = atol(argv[1]), rank, size;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	init(&dht, MPI_INFO_NULL, MPI_COMM_WORLD, N);

	int test_keys[] = {2, 435, 23, 54, 22, 65, 32, 56, 3244, 213};
	int test_size = sizeof(test_keys) / sizeof(test_keys[0]);

	if (rank == 0)
	{
		test_dht(&dht, test_size, test_keys, test_keys);
	}

	MPI_Finalize();

	deinit(&dht);
}