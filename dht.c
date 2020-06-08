#include <mpi.h>
#include <stdio.h>

int N;

int status = 0;

MPI_Win win;
MPI_Comm comm;
MPI_Info info;

int collisions;
int rank, size;
int *hashtable;

int hashtable_length;
int hashtable_length_per_p;

int hash(int key)
{
	return key % hashtable_length;
}

int get(int value)
{
	int retrieved = NULL;
	int destRank, destPos;
	int hash_key = hash(value);

	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;

	while (1)
	{
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
		status = MPI_Get(&retrieved, 1, MPI_INT, destRank, destPos, 1, MPI_INT, win);
		MPI_Win_unlock(destRank, win);

		if (retrieved == 0)
		{
			printf("%d Not found \n", value);
			return;
		}

		if (retrieved == value)
			break;

		hash_key++;
		destPos++;
		if (hash_key >= hashtable_length)
			hash_key = 0;
		if (destPos >= hashtable_length_per_p)
		{
			/* don't fall off the end, go to the next one */
			destPos = 0;
			destRank++;
			/* and for rank too */
			if (destRank >= size)
				destRank = 0;
		}
	}
	printf("Retrieved %d from process: %d position: %d \n", retrieved, destRank, destPos);
	return retrieved;
}

void insert(int key, int value)
{
	printf("Inserting %d\n", value);
	int destRank, destPos;

	int hash_key = hash(key);
	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;

	while (1)
	{
		int localhash = NULL;

		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
		status = MPI_Get(&localhash, 1, MPI_INT, destRank, destPos, 1, MPI_INT, win);
		MPI_Win_unlock(destRank, win);

		if (localhash)
		{
			// Another value already present
			printf("collission with value %d at process: %d  position: %d\n", localhash, destRank, destPos);
			hash_key++;
			destPos++;
			if (hash_key >= hashtable_length)
				hash_key = 0;
			if (destPos >= hashtable_length_per_p)
			{
				/* don't fall off the end, go to the next one */
				destPos = 0;
				destRank++;
				/* and for rank too */
				if (destRank >= size)
					destRank = 0;
			}
			collisions++;
		}
		else
		{
			MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
			MPI_Put(&value, 1, MPI_INT, destRank, destPos, 1, MPI_INT, win);
			MPI_Win_unlock(destRank, win);

			printf("Inserted %d process: %d position: %d\n", value, destRank, destPos);

			return;
		}
	}
}

int init_hashtable()
{
	MPI_Init(NULL, NULL);

	status = MPI_Info_create(&info);
	status = MPI_Info_set(info, "same_size", "true");

	comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

	hashtable_length = 2 * N + 1;
	hashtable_length_per_p = 2 * N / size + 1;

	hashtable = (int *)calloc(hashtable_length, sizeof(int));
	if (hashtable == NULL)
		exit(1);

	collisions = 0;
	status = MPI_Win_create(hashtable, hashtable_length_per_p * sizeof(int), sizeof(int), MPI_INFO_NULL, comm, &win);
	// printf("Process %d Hash Table inited Hash table lenth: %d, length per process %d \n", rank, hashtable_length, hashtable_length_per_p);
}

void test_dht(int no_test_cases, int test_keys[no_test_cases])
{
	for (int i = 0; i < no_test_cases; i++)
	{
		insert(test_keys[i], test_keys[i]);
		printf("-----------------------------------------------------------------------------\n");
	}
	printf("\n");
	for (int i = 0; i < no_test_cases; i++)
	{
		int retrieved = get(test_keys[i]);
		// printf("Retrieved %d",retrieved);
	}
}

int main(int argc, char **argv)
{
	N = atol(argv[1]);

	int sumCollisions = 0;
	init_hashtable();

	int test_cases[] = {2, 44, 7, 65, 1, 43, 28, 49, 34, 55, 18, 20, 39, 14};
	int test_size = sizeof(test_cases) / sizeof(test_cases[0]);

	if (rank == 0)
	{
		test_dht(test_size, test_cases);
	}

	MPI_Reduce(&collisions, &sumCollisions, 1, MPI_INT, MPI_SUM, 0, comm);
	if (rank == 0)
	{
		printf("\nNumber of collisions: %d\n", sumCollisions);
	}
	MPI_Finalize();
}