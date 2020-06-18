#include <mpi.h>
#include <stdio.h>
#include "test_cases.h"
#include <stdlib.h>
#include <time.h>


#define STOP_LISTENING 1
#define INSERT_MSG 2
#define GET_MSG 3
#define GET_ACK 4

MPI_Win win;
MPI_Comm comm;
MPI_Info info;

int N;
int status = 0;
int msg[4]; // (index 0 is type(break,insert,get,getsuccess)), 1 is key index, 2 is data index, 3 is destpos or (found status for getreply)
int success_msg;
int ch_collisions;
int rank, size;
struct HeadNode *hashtable;
int hashtable_length;
int hashtable_length_per_p;

struct Node {
  int key;
  int data;
  struct Node* next;
};

struct HeadNode{
    int index_type;  //0 for empty, 1 single, 2 chained
    int length;
    int key;
    int data;
    struct Node *next;
};

void insertAtEnd(struct Node** ref, int key, int data) {
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
  struct Node* last = *ref;

  new_node->data = data;
  new_node->key = key;
  new_node->next = NULL;

  if (*ref == NULL) {
    *ref = new_node;
    return;
  }

  while (last->next != NULL)
    last = last->next;

  last->next = new_node;
  return;
}

struct Node* insertAtBeginning(struct Node** ref, int key, int data) {
  // Allocate memory to a node
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));

  // insert the item
  new_node->data = data;
  new_node->key = key;
  new_node->next = NULL;

  // Move head to new node
  (*ref) = new_node;
  return new_node;
}

int ch_hash(int key)
{
	return key % hashtable_length;
}

int ch_get(int key)
{
	struct HeadNode *retrieved = (struct HeadNode*)malloc(sizeof(struct HeadNode));
	int destRank, destPos;
	int hash_key = ch_hash(key);
    int retrieved_value=0;

	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;


    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
    status = MPI_Get(retrieved,sizeof(struct Node), MPI_BYTE, destRank, destPos, sizeof(struct Node), MPI_BYTE, win);
    MPI_Win_unlock(destRank, win);

    if (retrieved->index_type == 0)
    {
        // printf("1. %d Not found \n", key);
        free(retrieved);
        return;
    }

    if (retrieved->key == key){
        // printf("Retrieved %d:%d from process: %d position: %d \n", retrieved->key,retrieved->data, destRank, destPos);
        free(retrieved);
        return;
    }

    if (retrieved -> index_type!=2){
        // printf("2. %d Not found \n", key);
        free(retrieved);
        return;
    }else{
        free(retrieved);
        msg[0]=GET_MSG;
        msg[1]=key;
        msg[3]=destPos;
        if(destRank!=rank){
            MPI_Send(msg, 4, MPI_INT, destRank, 0, comm);
            MPI_Recv(msg, 4, MPI_INT, destRank, 0, comm, MPI_STATUS_IGNORE);
        }else{
            overFlowRetrieve();
        }
        // if(msg[3]==1){
        //     printf("Retrieved %d:%d from process: %d position: %d \n",msg[1],msg[2], destRank, destPos);
        // }else{
        //     printf("3. %d Not found \n", key);
        // }
    }

}

void ch_insert(int key, int value)
{
	// printf("Inserting %d\n", value);
	int destRank, destPos;

	int hash_key = ch_hash(key);
	destRank = hash_key / hashtable_length_per_p;
	destPos = hash_key - hashtable_length_per_p * destRank;

    struct HeadNode *localhash = (struct HeadNode*)malloc(sizeof(struct HeadNode));

    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
    status = MPI_Get(localhash, sizeof(struct Node), MPI_BYTE, destRank, destPos, sizeof(struct Node), MPI_BYTE, win);
    MPI_Win_unlock(destRank, win);

    if (localhash->index_type==0)
    {
        localhash->index_type=1;
        localhash->key=key;
        localhash->data=value;
        localhash->length=1;

        MPI_Win_lock(MPI_LOCK_EXCLUSIVE, destRank, 0, win);
        MPI_Put(localhash, sizeof(struct Node), MPI_BYTE, destRank, destPos, sizeof(struct Node), MPI_BYTE, win);
        MPI_Win_unlock(destRank, win);

        // printf("Inserted %d process: %d position: %d\n", value, destRank, destPos);
        free(localhash);
        return;
    }
    

    // Another value already present
    // printf("collission of key:%d with key %d at process: %d  position: %d\n",key,localhash->key, destRank, destPos);
    free(localhash);
    msg[0]=INSERT_MSG;
    msg[1]=key;
    msg[2]=value;
    msg[3]=destPos;

    if(destRank!=rank){
        MPI_Send(msg, 4, MPI_INT, destRank, 0, comm);
        //uncomment if insert ACK is required.
        // MPI_Recv(&success_msg, 1, MPI_INT, destRank, 0, comm, MPI_STATUS_IGNORE); 
    }else{
        //local process inserts
        overFlowInsert();
    }
    ch_collisions++;
}

int ch_init_hashtable()
{
	MPI_Init(NULL, NULL);

	status = MPI_Info_create(&info);
	status = MPI_Info_set(info, "same_size", "true");

	comm = MPI_COMM_WORLD;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);

	hashtable_length = 2 * N + 1;
	hashtable_length_per_p = 2 * N / size + 1;

	hashtable = (struct HeadNode *)calloc(hashtable_length, sizeof(struct HeadNode));
	if (hashtable == NULL)
		exit(1);

	ch_collisions = 0;
	status = MPI_Win_create(hashtable, hashtable_length_per_p * sizeof(struct HeadNode), sizeof(struct HeadNode), MPI_INFO_NULL, comm, &win);
}

void ch_test_dht(int no_test_cases, int *keys, int *values)
{
    clock_t t;
    t = clock();
	for (int i = 0; i < no_test_cases; i++)
	{
		ch_insert(keys[i], values[i]);
		// printf("-----------------------------------------------------------------------------\n");
	}
	// printf("\n");
    double time_taken = ((double)t) / CLOCKS_PER_SEC;
    printf("ch insert time: %f ms\n", time_taken * 1000);
    
	t = clock();
	for (int i = 0; i < no_test_cases; i++)
	{
		int retrieved = ch_get(keys[i]);
	}
    t = clock() - t;
	time_taken = ((double)t) / CLOCKS_PER_SEC;
	printf("ch get time: %f ms\n", time_taken * 1000);

    //send msg end to all listening nodes
    msg[0]=1;
    for(int i=1;i<size;i++){
        MPI_Send(msg, 4, MPI_INT, i, 0, comm);
    }
}

void overFlowInsert(){
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);    
    if(hashtable[msg[3]].index_type==1){
        hashtable[msg[3]].index_type=2;
        struct Node * newnode = (struct Node*)malloc(sizeof(struct Node));
        newnode->key=msg[1];
        newnode->data=msg[2];
        newnode->next=NULL;
        hashtable[msg[3]].next=newnode;
    }else{
        hashtable[msg[3]].next = insertAtBeginning(&(hashtable[msg[3]].next),msg[1], msg[2]);
    }
    MPI_Win_unlock(rank, win);
    // printf("Chained Insert %d process: %d position: %d\n", msg[1], rank, msg[3]);
}

void overFlowRetrieve(){
    int found = 0;
    int data;
    MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, 0, win);
    struct Node* head = hashtable[msg[3]].next;
    while(head!=NULL){
        if(head->key == msg[1]){
            found=1;
            data=head-> data;
        }
        head = head-> next;
    }
    MPI_Win_unlock(rank, win);
    msg[0]=GET_ACK; msg[2]=data; msg[3]=found; // sendong found or not 
}

int onRecieve(){
    int msg_source = 0;
    MPI_Recv(msg, 4, MPI_INT, msg_source, 0, comm,
             MPI_STATUS_IGNORE); //source is 0
    
    if(msg[0]==STOP_LISTENING){
        return 0;
    }else if(msg[0]==INSERT_MSG){
        overFlowInsert();
        // success_msg=1;
        //uncomment if INSERT ACK required
        // MPI_Send(&success_msg, 1, MPI_INT, 0, 0, comm); // success ack end waiting at source
        return 1;
    }else if(msg[0]==GET_MSG){ 
        overFlowRetrieve();
        MPI_Send(msg, 4, MPI_INT, msg_source, 0, comm); //sending to source
        return 1;
    }
}

void freeallocated(){ //review this part for freeing memory
    for(int i=0;i<hashtable_length_per_p;i++){
        struct Node *head=hashtable[i].next;
        struct Node *temp;
        while(head!=NULL){
            temp=head->next;
            free(head);
            head=temp;
        }
    }
    free(hashtable);
}

int ch_get_collisions()
{
	int sum_collisions = 0;
	MPI_Reduce(&ch_collisions, &sum_collisions, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	return sum_collisions;
}

int main(int argc, char **argv)
{
	ch_init_hashtable();

    int N = atol(argv[1]);
	int unique_values = atol(argv[2]);
	int N_C = atol(argv[3]);

    int test_size = unique_values * N_C;
	int *test_keys = generate_collided_keys(N, unique_values, N_C);

	if (rank == 0)
	{
		ch_test_dht(test_size, test_keys,test_keys);
	}else{
        int msg_loop_break=1;
        while(msg_loop_break==1){
            msg_loop_break=onRecieve();
        }
    }

    int total_collisions=ch_get_collisions();
	if (rank == 0)
	{
		printf("\nNumber of collisions: %d\n", total_collisions);
	}
    freeallocated();
	MPI_Finalize();
}