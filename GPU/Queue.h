#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "Memory.h"

struct QueueNode
{
    struct QueueNode* next;
    struct QueueNode* prev;
    unsigned int data;
};

struct Queue
{
    struct QueueNode* root;
    unsigned int size;
    struct RAM* ram;
};

struct Queue Queue_create(struct RAM* ram)
{
    struct Queue new_queue;
    new_queue.root=NULL;
    new_queue.size=0;
    new_queue.ram=ram;
    return new_queue;
}

void Queue_push(struct Queue* queue,unsigned int data)
{
    struct QueueNode* new_node=(struct QueueNode*)RAM_allocate(queue->ram,sizeof(struct QueueNode));
    new_node->data=data;
    new_node->next=NULL;
    struct QueueNode** last=&(queue->root);
    for(;*last;last=&((*last)->next))
    {
        if(*last)
            new_node->prev=(*last);
    }
    *last=new_node;
    queue->size+=1;
}

void Queue_pop(struct Queue* queue)
{
    if(queue->size)
    {
        struct QueueNode* first=queue->root;
        queue->root=queue->root->next;
        if(queue->root)
            queue->root->prev=NULL;
        RAM_free(queue->ram,(char*)first,sizeof(struct QueueNode));
        queue->size-=1;
    }
}

unsigned int Queue_front(struct Queue* queue)
{
    if(queue->size)
        return queue->root->data;
    return 0;
}

#endif // QUEUE_H_INCLUDED
