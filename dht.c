#include <mpi.h>
#include <stdio.h>

int N;

int status = 0;

MPI_Win win;
MPI_Comm comm;
MPI_Info info;

int collisions = 0;
int rank, size;
int *hashtable;

int hashtable_length;
int hashtable_length_per_p;

int hash(int key)
{
	return key % hashtable_length;
}

void get(int value)
{
	int retrieved = NULL;
	int destRank, destPos;
	int hash_key = hash(value);

	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;

	// if(rank == destRank)
	// printf("temp: %d %d ",destRank,destPos);

	while (1)
	{
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
		status = MPI_Get(&retrieved, 1, MPI_INT, destRank, destPos, 1, MPI_INT, win);
		MPI_Win_unlock(destRank, win);

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
	printf("Retrieved %d pos: %d process: %d\n", retrieved, destPos, destRank);
	return retrieved;
}

void insert(int value)
{
	int destRank, destPos;

	int hash_key = hash(value);
	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;

	// printf("%d %d %d \n", hash, destRank, destPos);
	while (1)
	{
		int localhash = NULL;

		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
		status = MPI_Get(&localhash, 1, MPI_INT, destRank, destPos, 1, MPI_INT, win);
		MPI_Win_unlock(destRank, win);

		if (localhash)
		{
			// Another value already present
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

			printf("Inserted %d pos: %d process: %d\n", value, destPos, destRank);

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

	status = MPI_Win_create(hashtable, hashtable_length_per_p * sizeof(int), sizeof(int), MPI_INFO_NULL, comm, &win);
	printf("Process %d Hash Table inited Hash table lenth: %d, length per process %d \n", rank, hashtable_length, hashtable_length_per_p);
}

int main(int argc, char **argv)
{
	N = atol(argv[1]);

	init_hashtable();

	if (rank == 0)
	{	
		insert(3);
		insert(7);
		insert(22);
		insert(43);
		insert(64);

		printf("\n");

		get(3);
		get(7);
		get(22);
		get(43);
		get(64);
	}
	// Finalize the MPI environment.
	MPI_Finalize();
}