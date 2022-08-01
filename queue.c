#include "Queue.h"
#include <stdio.h>
#include <stdlib.h>


//on 11.0.2022
//implement queue syncronic functions
Node * createNode(void * data, int size){
    Node* node=(Node*)malloc(sizeof(Node));
    //node->data=malloc(size);
    node->data=data;
    node->next=NULL;
    //memcpy(node->data,data,size);
    //  exit(1);
    return node;
}

Queue* createQueue(int capacity){
    Queue *q=(Queue*)malloc(sizeof(Queue));
    initQueue(q,capacity);
    return q;
}
void initQueue(Queue* q,int capacity){
    q->capacity=capacity;
    q->size=0;
    q->head=NULL;
    q->tail=NULL;
    sem_init(&(q->mutex),0,1);//to do sync
    sem_init(&(q->is_full_mutex),0,capacity);//to check if there is place
    while(capacity)
    {
        sem_wait(&(q->is_full_mutex));
        capacity--;
    }
    return;
}
void freeNode(Node* node){
    if(node==NULL)
        return;
//    if(node->data!=N)
//    {free(node->data);
//    node->data=0;
//    }
    free(node);
    node=0;
}
void freeQueue(Queue* q){
    if(q==NULL)
        return;
    sem_destroy(&q->mutex);
    sem_destroy(&q->is_full_mutex);
    while(!isEmpty(q)){
        freeNode(dequeue(q));
    }
    free(q);
}
int getCapacity(Queue* q){
    return q->capacity;
}
int getSize(Queue* q){
    return q->size;
}
void enqueu(Queue* q,Node* data){

    sem_wait(&q->mutex);//lock sem

    if(q->size==q->capacity){
        return;
    }
    if(q->head==NULL){
        q->head=q->tail=data;
    }
    else{
        q->tail->next=data;
        q->tail=data;
    }
    q->size++;
    sem_post(&q->mutex);//free sem
    sem_post(&q->is_full_mutex);

}
Node* dequeue(Queue* q){
    Node* temp;
    sem_wait(&q->is_full_mutex);
    sem_wait(&q->mutex);//lock sem
    temp=q->head;
    q->head=q->head->next;
    q->size--;
    if(isEmpty(q))
        q->head=q->tail=NULL;
    sem_post(&q->mutex);//free sem
    return temp;
}
int isEmpty(Queue* q){
    return q->size==0;
}
int isFull(Queue *q){
    sem_wait(&q->mutex);//lock sem
    int isFull=q->size==q->capacity;
    sem_post(&q->mutex);//free sem
    return isFull;
}
