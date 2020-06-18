#include <stdio.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>
#include "cached-lp/cached-lp.h"
#include "linear-probing/linear-probing.h"
#include "test_cases.h"

void test_lp(struct LP_DHT *dht, int n, int *keys, int *values)
{
	for (int i = 0; i < n; i++)
	{
		assert(lp_insert(dht, keys[i], values[i]) == CLP_SUCCESS);
	}
	int value;

	clock_t t;
	t = clock();
	for (int i = 0; i < n; i++)
	{
		assert(lp_get(dht, keys[i], &value) == CLP_SUCCESS && value == values[i]);
	}
	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC;
	printf("LP get time: %f ms\n", time_taken * 1000);
}

void test_clp(struct CLP_DHT *dht, int n, int *keys, int *values)
{
	for (int i = 0; i < n; i++)
	{
		assert(clp_insert(dht, keys[i], values[i]) == CLP_SUCCESS);
	}
	int value;

	clock_t t;
	t = clock();
	for (int i = 0; i < n; i++)
	{
		assert(clp_get(dht, keys[i], &value) == CLP_SUCCESS && value == values[i]);
	}
	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC;
	printf("CLP get time: %f ms\n", time_taken * 1000);
}

int main(int argc, char **argv)
{
	struct CLP_DHT clp_dht;
	struct LP_DHT lp_dht;

	int N = atol(argv[1]), rank, size;
	int unique_values = atol(argv[2]);
	int N_C = atol(argv[3]);

	int clp_collisions, lp_collisions;

	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// int *test_keys = generate_random_keys(test_size);
	int test_size = unique_values * N_C;
	int *test_keys = generate_collided_keys(N, unique_values, N_C);

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