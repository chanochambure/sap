#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

struct QueueNode
{
    struct QueueNode* next;
    unsigned int data;
};

struct Queue
{
    struct QueueNode* root;
    unsigned int size;
};

__device__ struct Queue Queue_create()
{
    struct Queue new_queue;
    new_queue.root=NULL;
    new_queue.size=0;
    return new_queue;
}

__device__ void Queue_push(struct Queue* queue,unsigned int data)
{
    struct QueueNode* new_node=(struct QueueNode*)malloc(sizeof(struct QueueNode));
    new_node->data=data;
    new_node->next=NULL;
    struct QueueNode** last=&(queue->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    queue->size+=1;
}

__device__ void Queue_pop(struct Queue* queue)
{
    if(queue->size)
    {
        struct QueueNode* first=queue->root;
        queue->root=queue->root->next;
        free(first);
        queue->size-=1;
    }
}

__device__ unsigned int Queue_front(struct Queue* queue)
{
    if(queue->size)
        return queue->root->data;
    return 0;
}

#endif // QUEUE_H_INCLUDED
