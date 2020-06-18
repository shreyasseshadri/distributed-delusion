#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test_cases.h"

int test_keys_4_16[] = {2, 18, 34, 5, 21, 37, 53, 13, 29, 45, 61, 77};
int default_keys[] = {2, 44, 7, 65, 1, 43, 28, 49, 34, 55, 18, 20, 39, 14};

int *test_keys;
int *get_test_keys(int size, int N, int *test_size)
{
	if (size == 4 && N == 16)
	{
		test_keys = test_keys_4_16;
		*test_size = sizeof(test_keys_4_16) / sizeof(test_keys_4_16[0]);
	}
	else //default
	{
		test_keys = default_keys;
		*test_size = sizeof(default_keys) / sizeof(default_keys[0]);
	}
	return test_keys;
}

void print_array(int N, int *arr)
{
	for (int i = 0; i < N; i++)
		printf("%d ", arr[i]);
	printf("\n");
}

int *generate_collided_keys(int N, int unique_values, int N_C)
{
	assert(unique_values * N_C <= N);

	int in = 0;

	test_keys = calloc(unique_values * N_C, sizeof(int));
	for (int i = 1; i <= unique_values; i++)
	{
		for (int j = 0; j < N_C && in < unique_values * N_C; j++)
		{
			test_keys[in] = N * j + i;
			in++;
		}
	}

	assert(in == unique_values * N_C);
	// print_array(N_C* unique_values,test_keys);
	return test_keys;
}

int *generate_random_keys(int N)
{
	test_keys = calloc(N, sizeof(int));
	int M = 3 * N;
	int in, im;

	im = 0;
	srand(0);

	for (in = 0; in < N && im < M; ++in)
	{
		int rn = N - in;
		int rm = M - im;
		if (rand() % rn < rm)
			test_keys[im++] = in + 1;
	}

	// assert(im == M);
	// int lower = N;
	// int upper = 3 * N;
	// srand(0);
	// for (int i = 0; i < N; i++)
	// {
	// 	test_keys[i] = (rand() % (upper - lower + 1)) + lower;
	// }
	return test_keys;
}