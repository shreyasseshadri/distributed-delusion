int *test_keys;

int test_keys_4_16[] = {2, 18, 34, 5, 21, 37, 53, 13, 29, 45, 61, 77};

int* get_test_keys(int size,int N,int* test_size){
	if(size == 4 && N == 16)
	{
		test_keys = test_keys_4_16;
		*test_size =  sizeof(test_keys_4_16) / sizeof(test_keys_4_16[0]);
	}
	return test_keys;
}