#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "linear-probing.h"


#define UNOCCUPIED 0
#define OCCUPIED 1

int lp_collisions = 0;


int lp_hash(struct LP_DHT *dht, int key)
{
	return key % dht->ht_length;
}

int lp_get(struct LP_DHT *dht, int key, int *value)
{
	struct LP_Pair local_pair;
	int hash_key = lp_hash(dht, key);
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
				return LP_SUCCESS;
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
				return LP_FAILURE;
			}
		}
		else
		{
			return LP_FAILURE;
		}
	}
}

int lp_insert(struct LP_DHT *dht, int key, int value)
{
	struct LP_Pair local_pair;
	struct LP_Pair pair = {meta : OCCUPIED, key : key, value : value};
	int hash_key = lp_hash(dht, key);
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
			return LP_SUCCESS;
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
			return LP_FAILURE;
		}
		lp_collisions++;
	}
	return LP_FAILURE;
}

int lp_init(struct LP_DHT *dht, MPI_Info info, MPI_Comm comm, int table_size)
{
	int size;
	MPI_Comm_size(comm, &size);
	assert(table_size % size == 0);

	dht->n_process = size;
	dht->ht_length = table_size;
	dht->ht_length_per_process = table_size / size;
	dht->pairs = calloc(dht->ht_length_per_process, sizeof(struct LP_Pair));
	memset(dht->pairs, UNOCCUPIED, sizeof(struct LP_Pair) * dht->ht_length_per_process);

	MPI_Win_create(dht->pairs, dht->ht_length_per_process * sizeof(struct LP_Pair),
				   sizeof(struct LP_Pair), info, comm, &dht->store_win);
}

int lp_get_collisions()
{
	int sum_collisions = 0;
	MPI_Reduce(&lp_collisions, &sum_collisions, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	return sum_collisions;
}

int lp_deinit(struct LP_DHT *dht)
{	
	MPI_Win_free(&dht->store_win);
	free(dht->pairs);
}
