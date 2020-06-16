// Linked list operations in C

#include <stdio.h>
#include <stdlib.h>
struct Node {
  int key;
  int data;
  struct Node* next;
};

void insertAtEnd(struct Node** ref,int key, int data);
void deleteNode(struct Node** ref, int key);
void printList(struct Node* node);

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

void deleteNode(struct Node** ref, int key) {
  struct Node *temp = *ref, *prev;

  if (temp != NULL && temp->key == key) {
    *ref = temp->next;
    free(temp);
    return;
  }
  // Find the key to be deleted
  while (temp != NULL && temp->key != key) {
    prev = temp;
    temp = temp->next;
  }

  // If the key is not present
  if (temp == NULL) return;

  // Remove the node
  prev->next = temp->next;

  free(temp);
}

// Print the linked list
void printList(struct Node* node) {
  while (node != NULL) {
    printf(" %d %d\n", node->key,node->data);
    node = node->next;
  }
}

// Driver program

