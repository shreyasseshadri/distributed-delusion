#define LP_SUCCESS 0
#define LP_FAILURE 1

struct LP_Pair
{
	int meta;
	int key;
	int value;
};

struct LP_DHT
{
	struct LP_Pair *pairs;
	int n_process;
	int ht_length;
	int ht_length_per_process;
	int cache_length;
	MPI_Win store_win;
};

int lp_init(struct LP_DHT *dht, MPI_Info info, MPI_Comm comm, int table_size);

int lp_deinit(struct LP_DHT *dht);

int lp_get(struct LP_DHT *dht, int key, int *value);

int lp_insert(struct LP_DHT *dht, int key, int value);

int lp_get_collisions();
