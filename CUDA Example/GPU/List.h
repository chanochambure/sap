#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "Interval.h"

__device__ bool List_comp_increasing(CInterval interval,CInterval another_interval)
{
    if(interval.data[0]==another_interval.data[0])
        return interval.data[1]<another_interval.data[1];
    return interval.data[0]<another_interval.data[0];
}

__device__ bool List_comp_decreasing(CInterval interval,CInterval another_interval)
{
    if(interval.data[1]==another_interval.data[1])
        return interval.data[0]>another_interval.data[0];
    return interval.data[1]>another_interval.data[1];
}

struct ListNode
{
    ListNode* next;
    CInterval interval;
};

struct List
{
    ListNode* root;
    unsigned int size;
    bool sortType;
};

__device__ List List_create(bool sortType)
{
    List new_list;
    new_list.root=NULL;
    new_list.size=0;
    new_list.sortType=sortType;
    return new_list;
}

__device__ void List_clear(List* list)
{
    ListNode* node=list->root;
    while(node)
    {
        ListNode* temp=node->next;
        node->next=NULL;
		free(node);
        node=temp;
    }
    list->root=NULL;
    list->size=0;
}

__device__ void List_copy(List* to,List* from)
{
    List_clear(to);
    ListNode** node=&(to->root);
    for(ListNode* i=from->root;i;i=i->next)
    {
        *node=(ListNode*)malloc(sizeof(ListNode));
        (*node)->interval=i->interval;
        node=&((*node)->next);
    }
    to->size=from->size;
}

__device__ bool __List_private_find(CInterval interval,ListNode*** node,bool sortType)
{
    // Decreasing
    if(sortType)
    {
        for(;*(*node) && List_comp_decreasing((*(*node))->interval,interval);*node=&((*(*node))->next));
        return (*(*node) && Interval_equality((*(*node))->interval,interval));
    }
    // Increasing
    else
    {
        for(;*(*node) && List_comp_increasing((*(*node))->interval,interval);*node=&((*(*node))->next));
        return (*(*node) && Interval_equality((*(*node))->interval,interval));
    }
}

__device__ bool List_find(List* list,CInterval interval)
{
    ListNode** node=&list->root;
    return __List_private_find(interval,&node,list->sortType);
}

__device__ bool List_insert(List* list,CInterval interval)
{
    ListNode** position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
        return false;
    ListNode* new_node=(ListNode*)malloc(sizeof(ListNode));
    new_node->interval=interval;
    new_node->next=*position_node;
    *position_node=new_node;
    list->size+=1;
    return true;
}

__device__ bool List_remove(List* list,CInterval interval)
{
    ListNode** position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
    {
        list->size-=1;
        ListNode* temp=(*position_node)->next;
        (*position_node)->next=NULL;
        free(*position_node);
        *position_node=temp;
        return true;
    }
    return false;
}

#endif // LIST_H_INCLUDED
