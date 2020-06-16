#include "test_cases.h"

int test_keys_4_16[] = {2, 18, 34, 5, 21, 37, 53, 13, 29, 45, 61, 77};
int default_keys[] =  {2, 44, 7, 65, 1, 43, 28, 49, 34, 55, 18, 20, 39, 14};

int *test_keys;
int* get_test_keys(int size,int N,int* test_size){
	if(size == 4 && N == 16)
	{
		test_keys = test_keys_4_16;
		*test_size =  sizeof(test_keys_4_16) / sizeof(test_keys_4_16[0]);
	}
	else //default
	{
		test_keys = default_keys;
		*test_size =  sizeof(default_keys) / sizeof(default_keys[0]);
	}
	return test_keys;
}