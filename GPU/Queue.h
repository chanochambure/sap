#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include "Memory.h"

struct QueueNode
{
    QueueNode* next;
    QueueNode* prev;
    unsigned int data;
};

struct Queue
{
    QueueNode* root;
    unsigned int size;
    RAM* ram;
};

Queue Queue_create(RAM* ram)
{
    Queue new_queue;
    new_queue.root=NULL;
    new_queue.size=0;
    new_queue.ram=ram;
    return new_queue;
}

void Queue_push(Queue* queue,unsigned int data)
{
    QueueNode* new_node=(QueueNode*)RAM_allocate(queue->ram,sizeof(QueueNode));
    new_node->data=data;
    new_node->next=NULL;
    QueueNode** last=&(queue->root);
    for(;*last;last=&((*last)->next))
    {
        if(*last)
            new_node->prev=(*last);
    }
    *last=new_node;
    queue->size+=1;
}

void Queue_pop(Queue* queue)
{
    if(queue->size)
    {
        QueueNode* first=queue->root;
        queue->root=queue->root->next;
        if(queue->root)
            queue->root->prev=NULL;
        RAM_free(queue->ram,(char*)first,sizeof(QueueNode));
        queue->size-=1;
    }
}

unsigned int Queue_front(Queue* queue)
{
    if(queue->size)
        return queue->root->data;
    return 0;
}

#endif // QUEUE_H_INCLUDED
