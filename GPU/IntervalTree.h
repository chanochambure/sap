#ifndef INTERVALTREE_H_INCLUDED
#define INTERVALTREE_H_INCLUDED

#include "Memory.h"
#include "Interval.h"
#include "Stack.h"
#include "List.h"

struct IntervalTreeNode
{
    List increasing;
    List decreasing;
    IntervalTreeNode* left;
    IntervalTreeNode* right;
    float mid_point;
    int height;
    RAM* ram;
};

IntervalTreeNode IntervalTreeNode_create(RAM* ram)
{
    IntervalTreeNode node;
    node.increasing=List_create(ram,false);
    node.decreasing=List_create(ram,true);
    node.mid_point=0;
    node.height=0;
    node.left=NULL;
    node.right=NULL;
    node.ram=ram;
    return node;
}
IntervalTreeNode IntervalTreeNode_create(Interval interval,RAM* ram)
{
    IntervalTreeNode node;
    node.increasing=List_create(ram,false);
    node.decreasing=List_create(ram,true);
    List_insert(&(node.increasing),interval);
    List_insert(&(node.decreasing),interval);
    node.mid_point=Interval_get_mid_point(interval);
    node.height=1;
    node.left=NULL;
    node.right=NULL;
    node.ram=ram;
    return node;
}
int IntervalTreeNode_get_left_height(IntervalTreeNode* node)
{
    if(node->left)
        return node->left->height;
    return 0;
}
int IntervalTreeNode_get_right_height(IntervalTreeNode* node)
{
    if(node->right)
        return node->right->height;
    return 0;
}
void IntervalTreeNode_update_height(IntervalTreeNode* node)
{
    node->height=max(IntervalTreeNode_get_left_height(node),
                     IntervalTreeNode_get_right_height(node))+1;
}
void IntervalTreeNode_destroy_node(IntervalTreeNode* node)
{
    if(node->left)
        IntervalTreeNode_destroy_node(node->left);
    if(node->right)
        IntervalTreeNode_destroy_node(node->right);
    List_clear(&node->increasing);
    List_clear(&node->decreasing);
    node->left=NULL;
    node->right=NULL;
    RAM_free(node->ram,(char*)node,sizeof(IntervalTreeNode));
}

struct IntervalTree
{
    IntervalTreeNode* root;
    unsigned int size;
    RAM* ram;
};

void IntervalTree_clear(IntervalTree* itree)
{
    if(itree->root)
        IntervalTreeNode_destroy_node(itree->root);
    itree->root=NULL;
    itree->size=0;
}
void __IntervalTree_private_copy(RAM* ram,IntervalTreeNode** to,IntervalTreeNode* from)
{
    if(from)
    {
        (*to)=(IntervalTreeNode*)RAM_allocate(ram,sizeof(IntervalTreeNode));
        *(*to)=IntervalTreeNode_create(ram);
        List_copy(&((*to)->decreasing),&(from->decreasing));
        List_copy(&((*to)->increasing),&(from->increasing));
        (*to)->height=from->height;
        (*to)->mid_point=from->mid_point;
        __IntervalTree_private_copy(ram,&((*to)->left),from->left);
        __IntervalTree_private_copy(ram,&((*to)->right),from->right);
    }
}
void __IntervalTree_private_init(IntervalTree* itree,IntervalTreeNode* another_root,unsigned int new_size)
{
    if(another_root)
    {
        IntervalTree_clear(itree);
        __IntervalTree_private_copy(itree->ram,&(itree->root),another_root);
    }
    itree->size=new_size;
}

IntervalTree IntervalTree_create(RAM* ram)
{
    IntervalTree itree;
    itree.root=NULL;
    itree.size=0;
    itree.ram=ram;
    __IntervalTree_private_init(&itree,NULL,0);
    return itree;
}
IntervalTree IntervalTree_create(RAM* ram,IntervalTree* another_interval_tree)
{
    IntervalTree itree;
    itree.root=NULL;
    itree.size=0;
    itree.ram=ram;
    __IntervalTree_private_init(&itree,another_interval_tree->root,another_interval_tree->size);
    return itree;
}

IntervalTreeNode* __IntervalTree_private_balance(IntervalTreeNode* node);
IntervalTreeNode* __IntervalTree_private_get_overlaps(IntervalTreeNode* to,IntervalTreeNode* from);

IntervalTreeNode* __IntervalTree_private_delete_empty_node(IntervalTreeNode* target)
{
    if(!(target->left) && !(target->right))
    {
        IntervalTreeNode_destroy_node(target);
        return NULL;
    }
    if(!(target->left))
    {
        IntervalTreeNode* new_root=target->right;
        target->right=NULL;
        IntervalTreeNode_destroy_node(target);
        return new_root;
    }
    IntervalTreeNode* node=target->left;
    Stack stack=Stack_create(target->ram);
    while(node->right)
    {
        Stack_push(&stack,(char*)node);
        node=node->right;
    }
    if(stack.size)
    {
        IntervalTreeNode* top_node=(IntervalTreeNode*)Stack_top(&stack);
        top_node->right=node->left;
        node->left=target->left;
    }
    node->right=target->right;
    IntervalTreeNode* new_root=node;
    while(stack.size)
    {
        node=(IntervalTreeNode*)Stack_top(&stack);
        Stack_pop(&stack);
        if(stack.size)
        {
            IntervalTreeNode* top_node=(IntervalTreeNode*)Stack_top(&stack);
            top_node->right=__IntervalTree_private_get_overlaps(new_root,node);
        }
        else
            new_root->left=__IntervalTree_private_get_overlaps(new_root,node);
    }
    target->left=NULL;
    target->right=NULL;
    IntervalTreeNode_destroy_node(target);
    return __IntervalTree_private_balance(new_root);
}

IntervalTreeNode* __IntervalTree_private_get_overlaps(IntervalTreeNode* to,IntervalTreeNode* from)
{
    List temp_list;
    if(to->mid_point<from->mid_point)
    {
        temp_list=List_create(to->ram,false);
        for(ListNode* i=from->increasing.root;i;i=i->next)
        {
            Interval next=i->interval;
            if(Interval_is_right_of(next,to->mid_point))
                break;
            List_insert(&temp_list,next);
        }
    }
    else
    {
        temp_list=List_create(to->ram,true);
        for(ListNode* i=from->decreasing.root;i;i=i->next)
        {
            Interval next=i->interval;
            if(Interval_is_left_of(next,to->mid_point))
                break;
            List_insert(&temp_list,next);
        }
    }
    for(ListNode* i=temp_list.root;i;i=i->next)
    {
        Interval next=i->interval;
        List_remove(&(from->increasing),next);
        List_remove(&(from->decreasing),next);
    }
    for(ListNode* i=temp_list.root;i;i=i->next)
    {
        List_insert(&(to->increasing),i->interval);
        List_insert(&(to->decreasing),i->interval);
    }
    List_clear(&temp_list);
    if(!from->increasing.size)
        return __IntervalTree_private_delete_empty_node(from);
    return from;
}
IntervalTreeNode* __IntervalTree_private_left_rotate(IntervalTreeNode* node)
{
    IntervalTreeNode* head=node->right;
    node->right=head->left;
    head->left=node;
    IntervalTreeNode_update_height(node);
    head->left=__IntervalTree_private_get_overlaps(head,node);
    return head;
}
IntervalTreeNode* __IntervalTree_private_right_rotate(IntervalTreeNode* node)
{
    IntervalTreeNode* head=node->left;
    node->left=head->right;
    head->right=node;
    IntervalTreeNode_update_height(node);
    head->right=__IntervalTree_private_get_overlaps(head,node);
    return head;
}

IntervalTreeNode* __IntervalTree_private_balance(IntervalTreeNode* node)
{
    int status=IntervalTreeNode_get_left_height(node)-IntervalTreeNode_get_right_height(node);
    if(status<-1)
    {
        if(IntervalTreeNode_get_left_height(node->right)>IntervalTreeNode_get_right_height(node->right))
            node->right=__IntervalTree_private_right_rotate(node->right);
        return __IntervalTree_private_left_rotate(node);
    }
    else if(status>1)
    {
        if(IntervalTreeNode_get_right_height(node->left)>IntervalTreeNode_get_left_height(node->left))
            node->left=__IntervalTree_private_left_rotate(node->left);
        return __IntervalTree_private_right_rotate(node);
    }
    return node;
}
bool __IntervalTree_private_find(IntervalTreeNode* node,Interval interval)
{
    if(node)
    {
        if(Interval_contains(interval,node->mid_point))
        {
            if(List_find(&(node->increasing),interval))
                return true;
        }
        else if(Interval_is_left_of(interval,node->mid_point))
            return __IntervalTree_private_find(node->left,interval);
        else
            return __IntervalTree_private_find(node->right,interval);
    }
    return false;
}
IntervalTreeNode* __IntervalTree_private_insert(IntervalTree* itree,IntervalTreeNode* node,Interval interval)
{
    if(node==NULL)
    {
        itree->size+=1;
        IntervalTreeNode* new_node=(IntervalTreeNode*)RAM_allocate(itree->ram,sizeof(IntervalTreeNode));
        *new_node=IntervalTreeNode_create(interval,itree->ram);
        return new_node;
    }
    if(Interval_contains(interval,node->mid_point))
    {
        if(List_insert(&(node->decreasing),interval))
        {
            itree->size+=1;
            List_insert(&(node->increasing),interval);
        }
        return node;
    }
    else if(Interval_is_left_of(interval,node->mid_point))
        node->left=__IntervalTree_private_insert(itree,node->left,interval);
    else
        node->right=__IntervalTree_private_insert(itree,node->right,interval);
    IntervalTreeNode_update_height(node);
    return __IntervalTree_private_balance(node);
}
IntervalTreeNode* __IntervalTree_private_remove(IntervalTree* itree,IntervalTreeNode* node,Interval interval)
{
    if(node==NULL)
        return NULL;
    if(Interval_contains(interval,node->mid_point))
    {
        if(List_remove(&(node->decreasing),interval))
        {
            List_remove(&(node->increasing),interval);
            itree->size-=1;
            if(node->increasing.size==0)
                return __IntervalTree_private_delete_empty_node(node);
        }
    }
    else if(Interval_is_left_of(interval,node->mid_point))
        node->left=__IntervalTree_private_remove(itree,node->left,interval);
    else
        node->right=__IntervalTree_private_remove(itree,node->right,interval);
    IntervalTreeNode_update_height(node);
    return __IntervalTree_private_balance(node);
}
void __IntervalTree_private_range_query(List* data,IntervalTreeNode* node,Interval query)
{
    while(node)
    {
        if(Interval_contains(query,node->mid_point))
        {
            for(ListNode* i=node->increasing.root;i;i=i->next)
                List_insert(data,i->interval);
            __IntervalTree_private_range_query(data,node->left,query);
            __IntervalTree_private_range_query(data,node->right,query);
            break;
        }
        else if(Interval_is_left_of(query,node->mid_point))
        {
            for(ListNode* i=node->increasing.root;i;i=i->next)
            {
                if(!Interval_intersects(query,i->interval))
                    break;
                List_insert(data,i->interval);
            }
            node=node->left;
        }
        else if(Interval_is_right_of(query,node->mid_point))
        {
            for(ListNode* i=node->decreasing.root;i;i=i->next)
            {
                if(!Interval_intersects(query,i->interval))
                    break;
                List_insert(data,i->interval);
            }
            node=node->right;
        }
    }
}
bool IntervalTree_find(IntervalTree* itree,Interval interval)
{
    return __IntervalTree_private_find(itree->root,interval);
}
bool IntervalTree_insert(IntervalTree* itree,Interval interval)
{
    unsigned int last_size=itree->size;
    itree->root=__IntervalTree_private_insert(itree,itree->root,interval);
    return itree->size!=last_size;
}
bool IntervalTree_remove(IntervalTree* itree,Interval interval)
{
    if(itree->root)
    {
        unsigned int last_size=itree->size;
        itree->root=__IntervalTree_private_remove(itree,itree->root,interval);
        return itree->size!=last_size;
    }
    return false;
}
List IntervalTree_range_query(IntervalTree* itree,Interval query)
{
    List data=List_create(itree->ram,false);
    __IntervalTree_private_range_query(&data,itree->root,query);
    return data;
}

#endif // INTERVALTREE_H_INCLUDED
