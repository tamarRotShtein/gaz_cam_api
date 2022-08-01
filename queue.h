#ifndef QUEUE_H
#define QUEUE_H

#include  <semaphore.h>
typedef struct node{
    void* data;
    struct node* next;
}Node;

typedef struct queue{
    sem_t mutex;
    sem_t is_full_mutex;
    unsigned int size;
    unsigned int capacity;
    struct node* head;
    struct node* tail;
}Queue;

Node * createNode(void * data, int);
Queue* createQueue(int capacity);
void initQueue(Queue*,int capacity);
void freeNode(Node* node);
void freeQueue(Queue* q);
int getCapacity(Queue* q);
int getSize(Queue* q);
void enqueu(Queue* q,Node* data);
Node* dequeue(Queue* q);
int isEmpty(Queue* q);
int isFull(Queue *q);
#endif // QUEUE_H
