#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cached-lp.h"

#define CLP_UNOCCUPIED 0
#define CLP_OCCUPIED 1

int clp_collisions = 0;
int clp_hash(struct CLP_DHT *dht, int key)
{
	return key % dht->ht_length;
}

void increment(struct CLP_DHT *dht, int value, int *destPos, int *destRank)
{
	*destPos += value;
	if (*destPos >= dht->ht_length_per_process)
	{
		*destPos = 0;
		*destRank += 1;
		if (*destRank >= dht->n_process)
			*destRank = 0;
	}
}

int check_cache(int cache_length, struct CLP_Pair *cached_pairs, int key, int *value)
{
	for (int i = 0; i < cache_length; i++)
	{
		if (cached_pairs[i].key == key)
		{
			*value = cached_pairs[i].value;
			return CLP_SUCCESS;
		}
	}
	return CLP_FAILURE;
}

int clp_get(struct CLP_DHT *dht, int key, int *value)
{
	struct CLP_Pair local_pair;

	int hash_key = clp_hash(dht, key);
	int destRank = hash_key / dht->ht_length_per_process;
	int destPos = hash_key - dht->ht_length_per_process * destRank;
	int iniRank = destRank, iniPos = destPos;

	int CACHE_FLAG = 0;

	while (1)
	{
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
		MPI_Get(&local_pair, sizeof(struct CLP_Pair), MPI_BYTE,
				destRank, destPos,
				sizeof(struct CLP_Pair), MPI_BYTE, dht->store_win);
		MPI_Win_unlock(destRank, dht->store_win);

		if (local_pair.meta == CLP_OCCUPIED)
		{
			if (local_pair.key == key && CACHE_FLAG != 1)
			{

				*value = local_pair.value;
				return CLP_SUCCESS;
			}
			else
			{
				increment(dht, 1, &destPos, &destRank);

				int length_of_cache_to_bring = (destPos + dht->cache_length <= dht->ht_length_per_process)
												   ? dht->cache_length
												   : dht->ht_length_per_process - destPos;

				if (length_of_cache_to_bring <= 1)
				{
					CACHE_FLAG = 0;
					continue;
				}

				CACHE_FLAG = 1;
				struct CLP_Pair *cached_pairs = calloc(length_of_cache_to_bring, sizeof(struct CLP_Pair));

				MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
				MPI_Get(cached_pairs, length_of_cache_to_bring * sizeof(struct CLP_Pair), MPI_BYTE,
						destRank, destPos,
						length_of_cache_to_bring * sizeof(struct CLP_Pair), MPI_BYTE, dht->store_win);
				MPI_Win_unlock(destRank, dht->store_win);

				if (check_cache(length_of_cache_to_bring, cached_pairs, key, value) == CLP_SUCCESS)
				{
					free(cached_pairs);
					return CLP_SUCCESS;
				}
				increment(dht, length_of_cache_to_bring - 1, &destPos, &destRank);
				free(cached_pairs);
			}
			if (destRank == iniRank && destPos == iniPos)
			{
				return CLP_FAILURE;
			}
		}
		else
		{
			return CLP_FAILURE;
		}
	}
}

int clp_insert(struct CLP_DHT *dht, int key, int value)
{	
	struct CLP_Pair local_pair;
	struct CLP_Pair pair = {meta : CLP_OCCUPIED, key : key, value : value};
	int hash_key = clp_hash(dht, key);
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

		if (local_pair.meta == CLP_UNOCCUPIED)
		{
			MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, dht->store_win);
			MPI_Put(&pair, sizeof(pair), MPI_BYTE,
					destRank, destPos,
					sizeof(pair), MPI_BYTE, dht->store_win);
			MPI_Win_unlock(destRank, dht->store_win);
			return CLP_SUCCESS;
		}
		increment(dht, 1, &destPos, &destRank);
		if (destRank == iniRank && destPos == iniPos)
		{
			return CLP_FAILURE;
		}
		clp_collisions++;
	}
	return CLP_FAILURE;
}

int clp_init(struct CLP_DHT *dht, MPI_Info info, MPI_Comm comm, int table_size, int cache_length)
{
	int size;
	MPI_Comm_size(comm, &size);
	assert(table_size % size == 0);

	dht->n_process = size;
	dht->ht_length = table_size;
	dht->ht_length_per_process = table_size / size;
	dht->pairs = calloc(dht->ht_length_per_process, sizeof(struct CLP_Pair));
	dht->cache_length = cache_length;
	memset(dht->pairs, CLP_UNOCCUPIED, sizeof(struct CLP_Pair) * dht->ht_length_per_process);

	MPI_Win_create(dht->pairs, dht->ht_length_per_process * sizeof(struct CLP_Pair),
				   sizeof(struct CLP_Pair), info, comm, &dht->store_win);
}

int clp_get_collisions()
{
	int sum_collisions = 0;
	MPI_Reduce(&clp_collisions, &sum_collisions, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	return sum_collisions;
}

int clp_deinit(struct CLP_DHT *dht)
{	
	MPI_Win_free(&dht->store_win);
	free(dht->pairs);
}
