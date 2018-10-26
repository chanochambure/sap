#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

#include "Memory.h"

struct StackNode
{
    StackNode* next;
    StackNode* prev;
    char* data;
};

struct Stack
{
    StackNode* root;
    unsigned int size;
    RAM* ram;
};

Stack Stack_create(RAM* ram)
{
    Stack new_stack;
    new_stack.root=NULL;
    new_stack.size=0;
    new_stack.ram=ram;
    return new_stack;
}

void Stack_push(Stack* stack,char* data)
{
    StackNode* new_node=(StackNode*)RAM_allocate(stack->ram,sizeof(StackNode));
    new_node->data=data;
    new_node->next=NULL;
    StackNode** last=&(stack->root);
    for(;*last;last=&((*last)->next))
    {
        if(*last)
            new_node->prev=(*last);
    }
    *last=new_node;
    stack->size+=1;
}

void Stack_pop(Stack* stack)
{
    if(stack->size)
    {
        StackNode** last=&(stack->root);
        for(;(*last)->next;last=&((*last)->next));
        RAM_free(stack->ram,(char*)(*last),sizeof(StackNode));
        *last=NULL;
        stack->size-=1;
    }
}

char* Stack_top(Stack* stack)
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
