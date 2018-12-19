#ifndef INTERVALTREE_H_INCLUDED
#define INTERVALTREE_H_INCLUDED

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
};

__device__ IntervalTreeNode IntervalTreeNode_create()
{
    IntervalTreeNode node;
    node.increasing=List_create(false);
    node.decreasing=List_create(true);
    node.mid_point=0;
    node.height=0;
    node.left=NULL;
    node.right=NULL;
    return node;
}
__device__ IntervalTreeNode IntervalTreeNode_create(CInterval interval)
{
    IntervalTreeNode node;
    node.increasing=List_create(false);
    node.decreasing=List_create(true);
    List_insert(&(node.increasing),interval);
    List_insert(&(node.decreasing),interval);
    node.mid_point=Interval_get_mid_point(interval);
    node.height=1;
    node.left=NULL;
    node.right=NULL;
    return node;
}
__device__ int IntervalTreeNode_get_left_height(IntervalTreeNode* node)
{
    if(node->left)
        return node->left->height;
    return 0;
}
__device__ int IntervalTreeNode_get_right_height(IntervalTreeNode* node)
{
    if(node->right)
        return node->right->height;
    return 0;
}
__device__ void IntervalTreeNode_update_height(IntervalTreeNode* node)
{
    node->height=max(IntervalTreeNode_get_left_height(node),
                     IntervalTreeNode_get_right_height(node))+1;
}
__device__ void IntervalTreeNode_destroy_node(IntervalTreeNode* node)
{
    if(node->left)
        IntervalTreeNode_destroy_node(node->left);
    if(node->right)
        IntervalTreeNode_destroy_node(node->right);
    List_clear(&node->increasing);
    List_clear(&node->decreasing);
    node->left=NULL;
    node->right=NULL;
    free(node);
}

struct IntervalTree
{
    IntervalTreeNode* root;
    unsigned int size;
};

__device__ void IntervalTree_clear(IntervalTree* itree)
{
    if(itree->root)
        IntervalTreeNode_destroy_node(itree->root);
    itree->root=NULL;
    itree->size=0;
}

__device__ IntervalTree IntervalTree_create()
{
    IntervalTree itree;
    itree.root=NULL;
    itree.size=0;
    return itree;
}

__device__ IntervalTreeNode* __IntervalTree_private_balance(IntervalTreeNode* node);
__device__ IntervalTreeNode* __IntervalTree_private_get_overlaps(IntervalTreeNode* to,IntervalTreeNode* from);

__device__ IntervalTreeNode* __IntervalTree_private_delete_empty_node(IntervalTreeNode* target)
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
    Stack stack=Stack_create();
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
            if(!top_node->right->increasing.size)
                top_node->right=__IntervalTree_private_delete_empty_node(top_node->right);
        }
        else
        {
            new_root->left=__IntervalTree_private_get_overlaps(new_root,node);
            if(!new_root->left->increasing.size)
                new_root->left=__IntervalTree_private_delete_empty_node(new_root->left);
        }
    }
    target->left=NULL;
    target->right=NULL;
    IntervalTreeNode_destroy_node(target);
    return __IntervalTree_private_balance(new_root);
}

__device__ IntervalTreeNode* __IntervalTree_private_get_overlaps(IntervalTreeNode* to,IntervalTreeNode* from)
{
    List temp_list;
    if(to->mid_point<from->mid_point)
    {
        temp_list=List_create(false);
        for(ListNode* i=from->increasing.root;i;i=i->next)
        {
            CInterval next=i->interval;
            if(Interval_is_right_of(next,to->mid_point))
                break;
            List_insert(&temp_list,next);
        }
    }
    else
    {
        temp_list=List_create(true);
        for(ListNode* i=from->decreasing.root;i;i=i->next)
        {
            CInterval next=i->interval;
            if(Interval_is_left_of(next,to->mid_point))
                break;
            List_insert(&temp_list,next);
        }
    }
    for(ListNode* i=temp_list.root;i;i=i->next)
    {
        CInterval next=i->interval;
        List_remove(&(from->increasing),next);
        List_remove(&(from->decreasing),next);
    }
    for(ListNode* i=temp_list.root;i;i=i->next)
    {
        List_insert(&(to->increasing),i->interval);
        List_insert(&(to->decreasing),i->interval);
    }
    List_clear(&temp_list);
    return from;
}
//__device__ IntervalTreeNode* __IntervalTree_private_left_rotate(IntervalTreeNode* node)
//{
//    IntervalTreeNode* head=node->right;
//    node->right=head->left;
//    head->left=node;
//    IntervalTreeNode_update_height(node);
//    head->left=__IntervalTree_private_get_overlaps(head,node);
//    if(!head->left->increasing.size)
//        head->left=__IntervalTree_private_delete_empty_node(head->left);
//    return head;
//}
//__device__ IntervalTreeNode* __IntervalTree_private_right_rotate(IntervalTreeNode* node)
//{
//    IntervalTreeNode* head=node->left;
//    node->left=head->right;
//    head->right=node;
//    IntervalTreeNode_update_height(node);
//    head->right=__IntervalTree_private_get_overlaps(head,node);
//    if(!head->right->increasing.size)
//        head->right=__IntervalTree_private_delete_empty_node(head->right);
//    return head;
//}

__device__ IntervalTreeNode* __IntervalTree_private_balance(IntervalTreeNode* node)
{
    int status=IntervalTreeNode_get_left_height(node)-IntervalTreeNode_get_right_height(node);
    if(status<-1)
    {
        if(IntervalTreeNode_get_left_height(node->right)>IntervalTreeNode_get_right_height(node->right))
        {
            IntervalTreeNode* extra_node=node->right;
            IntervalTreeNode* head=extra_node->left;
            extra_node->left=head->right;
            head->right=extra_node;
            IntervalTreeNode_update_height(extra_node);
            head->right=__IntervalTree_private_get_overlaps(head,extra_node);
            if(!head->right->increasing.size)
                head->right=__IntervalTree_private_delete_empty_node(head->right);
            node->right=head;
        }
        IntervalTreeNode* extra_node=node;
        IntervalTreeNode* head=extra_node->right;
        extra_node->right=head->left;
        head->left=extra_node;
        IntervalTreeNode_update_height(extra_node);
        head->left=__IntervalTree_private_get_overlaps(head,extra_node);
        if(!head->left->increasing.size)
            head->left=__IntervalTree_private_delete_empty_node(head->left);
        return head;
    }
    else if(status>1)
    {
        if(IntervalTreeNode_get_right_height(node->left)>IntervalTreeNode_get_left_height(node->left))
        {
            IntervalTreeNode* extra_node=node->left;
            IntervalTreeNode* head=extra_node->right;
            extra_node->right=head->left;
            head->left=extra_node;
            IntervalTreeNode_update_height(extra_node);
            head->left=__IntervalTree_private_get_overlaps(head,extra_node);
            if(!head->left->increasing.size)
                head->left=__IntervalTree_private_delete_empty_node(head->left);
            node->left=head;
        }
        IntervalTreeNode* extra_node=node;
        IntervalTreeNode* head=extra_node->left;
        extra_node->left=head->right;
        head->right=extra_node;
        IntervalTreeNode_update_height(extra_node);
        head->right=__IntervalTree_private_get_overlaps(head,extra_node);
        if(!head->right->increasing.size)
            head->right=__IntervalTree_private_delete_empty_node(head->right);
        return head;
    }
    return node;
}
__device__ bool __IntervalTree_private_find(IntervalTreeNode* node,CInterval interval)
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
__device__ IntervalTreeNode* __IntervalTree_private_insert(IntervalTree* itree,IntervalTreeNode* node,CInterval interval)
{
    if(node==NULL)
    {
        itree->size+=1;
        IntervalTreeNode* new_node=(IntervalTreeNode*)malloc(sizeof(IntervalTreeNode));
        *new_node=IntervalTreeNode_create(interval);
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
__device__ IntervalTreeNode* __IntervalTree_private_remove(IntervalTree* itree,IntervalTreeNode* node,CInterval interval)
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
__device__ void __IntervalTree_private_range_query(List* data,IntervalTreeNode* node,CInterval query)
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
__device__ bool IntervalTree_find(IntervalTree* itree,CInterval interval)
{
    return __IntervalTree_private_find(itree->root,interval);
}
__device__ bool IntervalTree_insert(IntervalTree* itree,CInterval interval)
{
    unsigned int last_size=itree->size;
    itree->root=__IntervalTree_private_insert(itree,itree->root,interval);
    return itree->size!=last_size;
}
__device__ bool IntervalTree_remove(IntervalTree* itree,CInterval interval)
{
    if(itree->root)
    {
        unsigned int last_size=itree->size;
        itree->root=__IntervalTree_private_remove(itree,itree->root,interval);
        return itree->size!=last_size;
    }
    return false;
}
__device__ List IntervalTree_range_query(IntervalTree* itree,CInterval query)
{
    List data=List_create(false);
    __IntervalTree_private_range_query(&data,itree->root,query);
    return data;
}

#endif // INTERVALTREE_H_INCLUDED
