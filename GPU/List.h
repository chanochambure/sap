#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "Memory.h"
#include "Interval.h"

bool List_comp_increasing(Interval interval,Interval another_interval)
{
    if(interval.data[0]==another_interval.data[0])
        return interval.data[1]<another_interval.data[1];
    return interval.data[0]<another_interval.data[0];
}

bool List_comp_decreasing(Interval interval,Interval another_interval)
{
    if(interval.data[1]==another_interval.data[1])
        return interval.data[0]>another_interval.data[0];
    return interval.data[1]>another_interval.data[1];
}

struct ListNode
{
    ListNode* next;
    Interval interval;
};

struct List
{
    ListNode* root;
    unsigned int size;
    RAM* ram;
    bool sortType;
};

List List_create(RAM* ram,bool sortType)
{
    List new_list;
    new_list.root=NULL;
    new_list.size=0;
    new_list.ram=ram;
    new_list.sortType=sortType;
    return new_list;
}

void List_clear(List* list)
{
    ListNode* node=list->root;
    while(node)
    {
        ListNode* temp=node->next;
        node->next=NULL;
        RAM_free(list->ram,(char*)node,sizeof(ListNode));
        node=temp;
    }
    list->root=NULL;
    list->size=0;
}

void List_copy(List* to,List* from)
{
    List_clear(to);
    ListNode** node=&(to->root);
    for(ListNode* i=from->root;i;i=i->next)
    {
        *node=(ListNode*)RAM_allocate(to->ram,sizeof(ListNode));
        (*node)->interval=i->interval;
        node=&((*node)->next);
    }
    to->size=from->size;
}

bool __List_private_find(Interval interval,ListNode*** node,bool sortType)
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

bool List_find(List* list,Interval interval)
{
    ListNode** node=&list->root;
    return __List_private_find(interval,&node,list->sortType);
}

bool List_insert(List* list,Interval interval)
{
    ListNode** position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
        return false;
    ListNode* new_node=(ListNode*)RAM_allocate(list->ram,sizeof(ListNode));
    new_node->interval=interval;
    new_node->next=*position_node;
    *position_node=new_node;
    list->size+=1;
    return true;
}

bool List_remove(List* list,Interval interval)
{
    ListNode** position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
    {
        list->size-=1;
        ListNode* temp=(*position_node)->next;
        (*position_node)->next=NULL;
        RAM_free(list->ram,(char*)(*position_node),sizeof(ListNode));
        *position_node=temp;
        return true;
    }
    return false;
}

#endif // LIST_H_INCLUDED
