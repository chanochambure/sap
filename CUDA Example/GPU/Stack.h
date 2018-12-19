#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

struct StackNode
{
    StackNode* next;
    char* data;
};

struct Stack
{
    StackNode* root;
    unsigned int size;
};

__device__ Stack Stack_create()
{
    Stack new_stack;
    new_stack.root=NULL;
    new_stack.size=0;
    return new_stack;
}

__device__ void Stack_push(Stack* stack,char* data)
{
    StackNode* new_node=(StackNode*)malloc(sizeof(StackNode));
    new_node->data=data;
    new_node->next=NULL;
    StackNode** last=&(stack->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    stack->size+=1;
}

__device__ void Stack_pop(Stack* stack)
{
    if(stack->size)
    {
        StackNode** last=&(stack->root);
        for(;(*last)->next;last=&((*last)->next));
        free(*last);
        *last=NULL;
        stack->size-=1;
    }
}

__device__ char* Stack_top(Stack* stack)
{
    if(stack->size)
    {
        StackNode* last=stack->root;
        for(;last->next;last=last->next);
        return last->data;
    }
    return NULL;
}

#endif // STACK_H_INCLUDED
