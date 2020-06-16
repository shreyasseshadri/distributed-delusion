#define CLP_SUCCESS 0
#define CLP_FAILURE 1

#define CLP_UNOCCUPIED 0
#define CLP_OCCUPIED 1

struct Pair
{
	int meta;
	int key;
	int value;
};

struct CLP_DHT
{
	struct Pair *pairs;
	int n_process;
	int ht_length;
	int ht_length_per_process;
	int cache_length;
	MPI_Win store_win;
};

int clp_init(struct CLP_DHT *dht, MPI_Info info, MPI_Comm comm, int table_size, int cache_length);

int clp_deinit(struct CLP_DHT *dht);

int clp_get(struct CLP_DHT *dht, int key, int *value);

int clp_insert(struct CLP_DHT *dht, int key, int value);
