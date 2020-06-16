#include <stdio.h>
#include <mpi.h>
#include "cached-lp/cached-lp.h"
#include "linear-probing/linear-probing.h"
#include "test_cases.h"
#include <assert.h>

void test_lp(struct LP_DHT *dht, int n, int *keys, int *values)
{
	for (int i = 0; i < n; i++)
	{
		assert(lp_insert(dht, keys[i], values[i]) == CLP_SUCCESS);
	}
	int value;
	for (int i = 0; i < n; i++)
	{	
		assert(lp_get(dht, keys[i], &value) == CLP_SUCCESS && value == values[i]);
	}
}

void test_clp(struct CLP_DHT *dht, int n, int *keys, int *values)
{
	for (int i = 0; i < n; i++)
	{
		assert(clp_insert(dht, keys[i], values[i]) == CLP_SUCCESS);
	}
	int value;
	for (int i = 0; i < n; i++)
	{
		assert(clp_get(dht, keys[i], &value) == CLP_SUCCESS && value == values[i]);
	}
}

int main(int argc, char **argv)
{
	struct CLP_DHT clp_dht;
	struct LP_DHT lp_dht;

	int N = atol(argv[1]), rank, size;
	int clp_collisions, lp_collisions;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int test_size;
	int *test_keys = get_test_keys(size, N, &test_size);

	clp_init(&clp_dht, MPI_INFO_NULL, MPI_COMM_WORLD, N, 3);
	lp_init(&lp_dht, MPI_INFO_NULL, MPI_COMM_WORLD, N);

	if (rank == 0)
	{
		test_lp(&lp_dht, test_size, test_keys, test_keys);
		test_clp(&clp_dht, test_size, test_keys, test_keys);
	}

	clp_collisions = clp_get_collisions();
	lp_collisions = lp_get_collisions();

	if (rank == 0)
	{
		printf("CLP Collisions: %d\n", clp_collisions);
		printf("LP Collisions: %d\n", lp_collisions);
	}

	clp_deinit(&clp_dht);
	lp_deinit(&lp_dht);

	MPI_Finalize();
}