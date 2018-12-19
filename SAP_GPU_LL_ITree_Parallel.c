///MEMORY

#define NULL 0

struct RAM
{
    __global char* space;
    unsigned int startByte;
    unsigned int ramBytes;
    unsigned int totalBytes;
};

unsigned int RAM_getTotalBytes(unsigned int ramBytes)
{
    return ramBytes+ceil(ramBytes/8.0);
}

struct RAM RAM_create(__global char* space,unsigned int ramBytes)
{
    struct RAM newRam;
    newRam.space=NULL;
    newRam.ramBytes=newRam.startByte=newRam.totalBytes=0;
    if(ramBytes>0)
    {
        newRam.space=space;
        newRam.ramBytes=ramBytes;
        newRam.totalBytes=RAM_getTotalBytes(ramBytes);
        for(unsigned int i=0;i<newRam.totalBytes;++i)
            newRam.space[i]=0;
        newRam.startByte=newRam.totalBytes-newRam.ramBytes;
    }
    return newRam;
}

__global char* RAM_getDataByIndex(struct RAM* ram,unsigned int index)
{
    return &(ram->space[ram->startByte+index]);
}

unsigned int RAM_getIndexByData(struct RAM* ram,__global char* data)
{
    return (unsigned int)(data-ram->space)-ram->startByte;
}

__global char* RAM_allocate(struct RAM* ram,unsigned int bytes)
{
    if(bytes)
    {
        for(unsigned int i=0;i<ram->ramBytes;++i)
        {
            unsigned int j=0;
            while(j<bytes)
            {
                unsigned int ByteId=(i+j)/8;
                unsigned int bitId=(i+j)%8;
                if(ram->space[ByteId] & 1<<bitId)
                    break;
                ++j;
            }
            if(j==bytes)
            {
                for(unsigned int k=0;k<bytes;++k)
                {
                    unsigned int ByteId=(i+k)/8;
                    unsigned int bitId=(i+k)%8;
                    ram->space[ByteId] |= 1<<bitId;
                }
                return RAM_getDataByIndex(ram,i);
            }
        }
    }
    return NULL;
}

void RAM_free(struct RAM* ram,__global char* data,unsigned int bytes)
{
    unsigned int index=RAM_getIndexByData(ram,data);
    for(unsigned int i=0;i<bytes;++i)
    {
        unsigned int ByteId=(index+i)/8;
        unsigned int bitId=(index+i)%8;
        ram->space[ByteId] &= ~(1<<bitId);
    }
}
///MEMORY

///QUEUE
struct QueueNode
{
    __global struct QueueNode* next;
    unsigned int data;
};

struct Queue
{
    __global struct QueueNode* root;
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

void Queue_push(__global struct Queue* queue,unsigned int data)
{
    __global struct QueueNode* new_node=(__global struct QueueNode*)RAM_allocate(queue->ram,sizeof(struct QueueNode));
    new_node->data=data;
    new_node->next=NULL;
    __global struct QueueNode* __global * last=&(queue->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    queue->size+=1;
}

void Queue_pop(__global struct Queue* queue)
{
    if(queue->size)
    {
        __global struct QueueNode* first=queue->root;
        queue->root=queue->root->next;
        RAM_free(queue->ram,(__global char*)first,sizeof(struct QueueNode));
        queue->size-=1;
    }
}

unsigned int Queue_front(__global struct Queue* queue)
{
    if(queue->size)
        return queue->root->data;
    return 0;
}
///QUEUE

///RADIX SORT
__global float* CreateArray(struct RAM* ram,__global float* numbers,unsigned int n,bool x_y,int begin)
{
    int axis=(x_y)?3:1;
    __global float* return_value=(__global float*)RAM_allocate(ram,n*3*sizeof(float));
    for(unsigned int i=0;i<n;++i)
    {
        return_value[i+(2*n)]=numbers[(begin+i)*5];
        return_value[i]=numbers[(begin+i)*5+axis];
        return_value[i+n]=numbers[(begin+i)*5+axis+1];
    }
    return return_value;
}

__global unsigned short* RadixSort(struct RAM* ram,__global float* points,unsigned short size)
{
    __global unsigned short* index=(__global unsigned short*)RAM_allocate(ram,size*sizeof(unsigned short));
    for(unsigned short i=0;i<size;++i)
        index[i]=i;
    int max_value=points[0];
    for(unsigned int i=1;i<size;++i)
    {
    	if(max_value<points[i])
    		max_value=points[i];
    }
    unsigned int cifras = 0;
    while(max_value)
    {
        ++cifras;
        max_value/=10;
    }
    __global struct Queue* queues=(__global struct Queue*)RAM_allocate(ram,10*sizeof(struct Queue));
    for(unsigned int i=0;i<10;++i)
        queues[i]=Queue_create(ram);
    for(unsigned int i=0;i<cifras;i++)
    {
    	for(unsigned int j=0;j<size;++j)
    	{
    		int d = (int)(points[index[j]]/pow((float)10,(float)i))%10;
    		Queue_push(queues+d,index[j]);
    	}
    	int p=0;
    	for(unsigned int j=0;j<10;++j)
    	{
    		while(queues[j].size)
    		{
    			index[p]=Queue_front(&(queues[j]));
    			Queue_pop(queues+j);
    			++p;
    		}
    	}
    }
    RAM_free(ram,(__global char*)queues,10*sizeof(struct Queue));
    return index;
}
///RADIX SORT

///INTERVAL
struct Interval
{
    float data_left;
    float data_right;
};

struct Interval Interval_create(float left,float right)
{
    struct Interval newInterval;
    if(left>right)
    {
        newInterval.data_right=left;
        newInterval.data_left=right;
    }
    else
    {
        newInterval.data_left=left;
        newInterval.data_right=right;
    }
    return newInterval;
}
float Interval_get_mid_point(struct Interval interval)
{
    return (interval.data_left+interval.data_right)/2.0;
}
bool Interval_contains_p(struct Interval interval,float point)
{
    return interval.data_left<=point && point<=interval.data_right;
}
bool Interval_contains(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left<=another_interval.data_left && another_interval.data_right<=interval.data_right;
}
bool Interval_intersects(struct Interval interval,struct Interval another_interval)
{
    float ini_point=max(interval.data_left,another_interval.data_left);
    float end_point=min(interval.data_right,another_interval.data_right);
    return ini_point<=end_point;
}
bool Interval_is_left_of_p(struct Interval interval,float point)
{
    return interval.data_right<point;
}
bool Interval_is_right_of_p(struct Interval interval,float point)
{
    return point<interval.data_left;
}
bool Interval_is_left_of(struct Interval interval,struct Interval another_interval)
{
    return Interval_is_left_of_p(interval,another_interval.data_left);
}
bool Interval_is_right_of(struct Interval interval,struct Interval another_interval)
{
    return Interval_is_right_of_p(interval,another_interval.data_right);
}
bool Interval_equality(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left==another_interval.data_left && interval.data_right==another_interval.data_right;
}
bool Interval_difference(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left!=another_interval.data_left || interval.data_right!=another_interval.data_right;
}
///INTERVAL

///LIST
bool List_comp_increasing(struct Interval interval,struct Interval another_interval)
{
    if(interval.data_left==another_interval.data_left)
        return interval.data_right<another_interval.data_right;
    return interval.data_left<another_interval.data_left;
}

bool List_comp_decreasing(struct Interval interval,struct Interval another_interval)
{
    if(interval.data_right==another_interval.data_right)
        return interval.data_left>another_interval.data_left;
    return interval.data_right>another_interval.data_right;
}

struct ListNode
{
    __global struct ListNode* next;
    struct Interval interval;
};

struct List
{
    __global struct ListNode* root;
    unsigned int size;
    struct RAM* ram;
    bool sortType;
};

__global struct List* List_create(struct RAM* ram,bool sortType)
{
    __global struct List* new_list=(__global struct List*)RAM_allocate(ram,sizeof(struct List));
    new_list->root=NULL;
    new_list->size=0;
    new_list->ram=ram;
    new_list->sortType=sortType;
    return new_list;
}

void List_clear(__global struct List* list)
{
    __global struct ListNode* node=list->root;
    while(node)
    {
        __global struct ListNode* temp=node->next;
        node->next=NULL;
        RAM_free(list->ram,(__global char*)node,sizeof(struct ListNode));
        node=temp;
    }
    list->root=NULL;
    list->size=0;
}

void List_copy(__global struct List* to,__global struct List* from)
{
    List_clear(to);
    __global struct ListNode* __global* node=&(to->root);
    for(__global struct ListNode* i=from->root;i;i=i->next)
    {
        *node=(__global struct ListNode*)RAM_allocate(to->ram,sizeof(struct ListNode));
        (*node)->interval=i->interval;
        node=&((*node)->next);
    }
    to->size=from->size;
}

bool __List_private_find(struct Interval interval,__global struct ListNode* __global** node,bool sortType)
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

bool List_find(__global struct List* list,struct Interval interval)
{
    __global struct ListNode* __global* node=&list->root;
    return __List_private_find(interval,&node,list->sortType);
}

bool List_insert(__global struct List* list,struct Interval interval)
{
    __global struct ListNode* __global* position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
        return false;
    __global struct ListNode* new_node=(__global struct ListNode*)RAM_allocate(list->ram,sizeof(struct ListNode));
    new_node->interval=interval;
    new_node->next=*position_node;
    *position_node=new_node;
    list->size+=1;
    return true;
}

bool List_remove(__global struct List* list,struct Interval interval)
{
    __global struct ListNode* __global* position_node=&list->root;
    if(__List_private_find(interval,&position_node,list->sortType))
    {
        list->size-=1;
        __global struct ListNode* temp=(*position_node)->next;
        (*position_node)->next=NULL;
        RAM_free(list->ram,(__global char*)(*position_node),sizeof(struct ListNode));
        *position_node=temp;
        return true;
    }
    return false;
}

void List_delete(__global struct List* list,struct RAM* ram)
{
    List_clear(list);
    RAM_free(ram,(__global char*)list,sizeof(struct List));
}
///LIST

///INTERVALTREE

struct IntervalTreeNode
{
    __global struct List* increasing;
    __global struct List* decreasing;
    __global struct IntervalTreeNode* left;
    __global struct IntervalTreeNode* right;
    float mid_point;
    int height;
    struct RAM* ram;
};

__global struct IntervalTreeNode* ITN_create_empty(struct RAM* ram)
{
    __global struct IntervalTreeNode* node=(__global struct IntervalTreeNode*)RAM_allocate(ram,sizeof(struct IntervalTreeNode));
    node->increasing=List_create(ram,false);
    node->decreasing=List_create(ram,true);
    node->mid_point=0;
    node->height=0;
    node->left=NULL;
    node->right=NULL;
    node->ram=ram;
    return node;
}
__global struct IntervalTreeNode* ITN_create(struct Interval interval,struct RAM* ram)
{
    __global struct IntervalTreeNode* node=(__global struct IntervalTreeNode*)RAM_allocate(ram,sizeof(struct IntervalTreeNode));
    node->increasing=List_create(ram,false);
    node->decreasing=List_create(ram,true);
    List_insert(node->increasing,interval);
    List_insert(node->decreasing,interval);
    node->mid_point=Interval_get_mid_point(interval);
    node->height=1;
    node->left=NULL;
    node->right=NULL;
    node->ram=ram;
    return node;
}
int ITN_get_left_height(__global struct IntervalTreeNode* node)
{
    if(node->left)
        return node->left->height;
    return 0;
}
int ITN_get_right_height(__global struct IntervalTreeNode* node)
{
    if(node->right)
        return node->right->height;
    return 0;
}
void ITN_update_height(__global struct IntervalTreeNode* node)
{
    node->height=max(ITN_get_left_height(node),
                     ITN_get_right_height(node))+1;
}
void ITN_destroy_node(__global struct IntervalTreeNode* node)
{
    if(node->left)
        ITN_destroy_node(node->left);
    if(node->right)
        ITN_destroy_node(node->right);
    List_delete(node->increasing,node->ram);
    List_delete(node->decreasing,node->ram);
    node->increasing=NULL;
    node->decreasing=NULL;
    node->left=NULL;
    node->right=NULL;
    RAM_free(node->ram,(__global char*)node,sizeof(struct IntervalTreeNode));
}

///STACK
struct StackNode
{
    __global struct StackNode* next;
    __global struct IntervalTreeNode* data;
};

struct Stack
{
    __global struct StackNode* root;
    unsigned int size;
    struct RAM* ram;
};

__global struct Stack* Stack_create(struct RAM* ram)
{
    __global struct Stack* new_stack=(__global struct Stack*)RAM_allocate(ram,sizeof(struct Stack));
    new_stack->root=NULL;
    new_stack->size=0;
    new_stack->ram=ram;
    return new_stack;
}

void Stack_delete(__global struct Stack* stack,struct RAM* ram)
{
    RAM_free(ram,(__global char*)stack,sizeof(struct Stack));
}

void Stack_push(__global struct Stack* stack,__global struct IntervalTreeNode* data)
{
    __global struct StackNode* new_node=(__global struct StackNode*)RAM_allocate(stack->ram,sizeof(struct StackNode));
    new_node->data=data;
    new_node->next=NULL;
    __global struct StackNode* __global* last=&(stack->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    stack->size+=1;
}

void Stack_pop(__global struct Stack* stack)
{
    if(stack->size)
    {
        __global struct StackNode* __global* last=&(stack->root);
        for(;(*last)->next;last=&((*last)->next));
        RAM_free(stack->ram,(__global char*)(*last),sizeof(struct StackNode));
        *last=NULL;
        stack->size-=1;
    }
}

__global struct IntervalTreeNode* Stack_top(__global struct Stack* stack)
{
    if(stack->size)
    {
        __global struct StackNode* last=stack->root;
        for(;last->next;last=last->next);
        return last->data;
    }
    return 0;
}
///STACK

struct IntervalTree
{
    __global struct IntervalTreeNode* root;
    struct RAM* ram;
};

void IT_clear(__global struct IntervalTree* itree)
{
    if(itree->root)
        ITN_destroy_node(itree->root);
    itree->root=NULL;
}

__global struct IntervalTree* IT_create(struct RAM* ram)
{
    __global struct IntervalTree* itree=(__global struct IntervalTree*)RAM_allocate(ram,sizeof(struct IntervalTree));
    itree->root=NULL;
    itree->ram=ram;
    return itree;
}

void IT_delete(__global struct IntervalTree* itree,struct RAM* ram)
{
    IT_clear(itree);
    RAM_free(ram,(__global char*)itree,sizeof(struct IntervalTree));
}

__global struct IntervalTreeNode* __IT_pr_get_overlaps(__global struct IntervalTreeNode* to,__global struct IntervalTreeNode* from);

__global struct IntervalTreeNode* __IT_pr_balance(__global struct IntervalTreeNode* node)
{
    int status=ITN_get_left_height(node)-ITN_get_right_height(node);
    if(status<-1)
    {
        if(ITN_get_left_height(node->right)>ITN_get_right_height(node->right))
        {
            __global struct IntervalTreeNode* current_node=node->right;
            __global struct IntervalTreeNode* head=current_node->left;
            current_node->left=head->right;
            head->right=current_node;
            ITN_update_height(current_node);
            head->right=__IT_pr_get_overlaps(head,current_node);
            node->right=head;
        }
        __global struct IntervalTreeNode* head=node->right;
        node->right=head->left;
        head->left=node;
        ITN_update_height(node);
        head->left=__IT_pr_get_overlaps(head,node);
        return head;
    }
    else if(status>1)
    {
        if(ITN_get_right_height(node->left)>ITN_get_left_height(node->left))
        {
            __global struct IntervalTreeNode* current_node=node->left;
            __global struct IntervalTreeNode* head=current_node->right;
            current_node->right=head->left;
            head->left=current_node;
            ITN_update_height(current_node);
            head->left=__IT_pr_get_overlaps(head,current_node);
            node->left=head;
        }
        __global struct IntervalTreeNode* head=node->left;
        node->left=head->right;
        head->right=node;
        ITN_update_height(node);
        head->right=__IT_pr_get_overlaps(head,node);
        return head;
    }
    return node;
}

__global struct IntervalTreeNode* __IT_pr_get_overlaps(__global struct IntervalTreeNode* to,__global struct IntervalTreeNode* from)
{
    __global struct List* temp_list=NULL;
    if(to->mid_point<from->mid_point)
    {
        temp_list=List_create(to->ram,false);
        for(__global struct ListNode* i=from->increasing->root;i;i=i->next)
        {
            struct Interval next=i->interval;
            if(Interval_is_right_of_p(next,to->mid_point))
                break;
            List_insert(temp_list,next);
        }
    }
    else
    {
        temp_list=List_create(to->ram,true);
        for(__global struct ListNode* i=from->decreasing->root;i;i=i->next)
        {
            struct Interval next=i->interval;
            if(Interval_is_left_of_p(next,to->mid_point))
                break;
            List_insert(temp_list,next);
        }
    }
    for(__global struct ListNode* i=temp_list->root;i;i=i->next)
    {
        struct Interval next=i->interval;
        List_remove(from->increasing,next);
        List_remove(from->decreasing,next);
    }
    for(__global struct ListNode* i=temp_list->root;i;i=i->next)
    {
        List_insert(to->increasing,i->interval);
        List_insert(to->decreasing,i->interval);
    }
    List_delete(temp_list,to->ram);
    if(!from->increasing->size)
    {
        __global struct IntervalTreeNode* target=from;
        if(!(target->left) && !(target->right))
        {
            ITN_destroy_node(target);
            return NULL;
        }
        if(!(target->left))
        {
            __global struct IntervalTreeNode* new_root=target->right;
            target->right=NULL;
            ITN_destroy_node(target);
            return new_root;
        }
        __global struct IntervalTreeNode* node=target->left;
        __global struct Stack* stack=Stack_create(target->ram);
        while(node->right)
        {
            Stack_push(stack,node);
            node=node->right;
        }
        if(stack->size)
        {
            __global struct IntervalTreeNode* top_node=Stack_top(stack);
            top_node->right=node->left;
            node->left=target->left;
        }
        node->right=target->right;
        __global struct IntervalTreeNode* new_root=node;
        while(stack->size)
        {
            node=Stack_top(stack);
            Stack_pop(stack);
            if(stack->size)
            {
                __global struct IntervalTreeNode* top_node=Stack_top(stack);
                top_node->right=__IT_pr_get_overlaps(new_root,node);
            }
            else
                new_root->left=__IT_pr_get_overlaps(new_root,node);
        }
        Stack_delete(stack,target->ram);
        target->left=NULL;
        target->right=NULL;
        ITN_destroy_node(target);
        return __IT_pr_balance(new_root);
    }
    return from;
}

bool __IT_pr_find(__global struct IntervalTreeNode* node,struct Interval interval)
{
    if(node)
    {
        if(Interval_contains_p(interval,node->mid_point))
        {
            if(List_find(node->increasing,interval))
                return true;
        }
        else if(Interval_is_left_of_p(interval,node->mid_point))
            return __IT_pr_find(node->left,interval);
        else
            return __IT_pr_find(node->right,interval);
    }
    return false;
}

__global struct IntervalTreeNode* __IT_pr_insert(__global struct IntervalTree* itree,__global struct IntervalTreeNode* node,struct Interval interval)
{
    if(node==NULL)
    {
        __global struct IntervalTreeNode* new_node=ITN_create(interval,itree->ram);
        return new_node;
    }
    if(Interval_contains_p(interval,node->mid_point))
    {
        if(List_insert(node->decreasing,interval))
        {
            List_insert(node->increasing,interval);
        }
        return node;
    }
    else if(Interval_is_left_of_p(interval,node->mid_point))
        node->left=__IT_pr_insert(itree,node->left,interval);
    else
        node->right=__IT_pr_insert(itree,node->right,interval);
    ITN_update_height(node);
    return __IT_pr_balance(node);
}

__global struct IntervalTreeNode* __IT_pr_remove(__global struct IntervalTree* itree,__global struct IntervalTreeNode* node,struct Interval interval)
{
    if(node==NULL)
        return NULL;
    if(Interval_contains_p(interval,node->mid_point))
    {
        if(List_remove(node->decreasing,interval))
        {
            List_remove(node->increasing,interval);
            if(node->increasing->size==0)
            {
                __global struct IntervalTreeNode* target=node;
                if(!(target->left) && !(target->right))
                {
                    ITN_destroy_node(target);
                    return NULL;
                }
                if(!(target->left))
                {
                    __global struct IntervalTreeNode* new_root=target->right;
                    target->right=NULL;
                    ITN_destroy_node(target);
                    return new_root;
                }
                __global struct IntervalTreeNode* extra_node=target->left;
                __global struct Stack* stack=Stack_create(target->ram);
                while(extra_node->right)
                {
                    Stack_push(stack,extra_node);
                    extra_node=extra_node->right;
                }
                if(stack->size)
                {
                    __global struct IntervalTreeNode* top_node=Stack_top(stack);
                    top_node->right=extra_node->left;
                    extra_node->left=target->left;
                }
                extra_node->right=target->right;
                __global struct IntervalTreeNode* new_root=extra_node;
                while(stack->size)
                {
                    extra_node=Stack_top(stack);
                    Stack_pop(stack);
                    if(stack->size)
                    {
                        __global struct IntervalTreeNode* top_node=Stack_top(stack);
                        top_node->right=__IT_pr_get_overlaps(new_root,extra_node);
                    }
                    else
                        new_root->left=__IT_pr_get_overlaps(new_root,extra_node);
                }
                Stack_delete(stack,target->ram);
                target->left=NULL;
                target->right=NULL;
                ITN_destroy_node(target);
                return __IT_pr_balance(new_root);
            }
        }
    }
    else if(Interval_is_left_of_p(interval,node->mid_point))
        node->left=__IT_pr_remove(itree,node->left,interval);
    else
        node->right=__IT_pr_remove(itree,node->right,interval);
    ITN_update_height(node);
    return __IT_pr_balance(node);
}

void __IT_pr_range_query(__global struct List* data,__global struct IntervalTreeNode* node,struct Interval query)
{
    while(node)
    {
        if(Interval_contains_p(query,node->mid_point))
        {
            for(__global struct ListNode* i=node->increasing->root;i;i=i->next)
                List_insert(data,i->interval);
            __IT_pr_range_query(data,node->left,query);
            __IT_pr_range_query(data,node->right,query);
            break;
        }
        else if(Interval_is_left_of_p(query,node->mid_point))
        {
            for(__global struct ListNode* i=node->increasing->root;i;i=i->next)
            {
                if(!Interval_intersects(query,i->interval))
                    break;
                List_insert(data,i->interval);
            }
            node=node->left;
        }
        else if(Interval_is_right_of_p(query,node->mid_point))
        {
            for(__global struct ListNode* i=node->decreasing->root;i;i=i->next)
            {
                if(!Interval_intersects(query,i->interval))
                    break;
                List_insert(data,i->interval);
            }
            node=node->right;
        }
    }
}

bool IT_find(__global struct IntervalTree* itree,struct Interval interval)
{
    return __IT_pr_find(itree->root,interval);
}
void IT_insert(__global struct IntervalTree* itree,struct Interval interval)
{
    itree->root=__IT_pr_insert(itree,itree->root,interval);
}
void IT_remove(__global struct IntervalTree* itree,struct Interval interval)
{
    if(itree->root)
        itree->root=__IT_pr_remove(itree,itree->root,interval);
}
__global struct List* IT_range_query(__global struct IntervalTree* itree,struct Interval query)
{
    __global struct List* data=List_create(itree->ram,false);
    __IT_pr_range_query(data,itree->root,query);
    return data;
}

///INTERVALTREE

unsigned int get_id(unsigned int i,unsigned int j,unsigned int size)
{
    unsigned int new_index=0;
    if(i>j)
    {
        for(unsigned int k=size-1;k>i;--k)
            new_index+=k;
        return new_index+j;
    }
    else
    {
        for(unsigned int k=size-1;k>j;--k)
            new_index+=k;
        return new_index+i;
    }
}

///KERNEL
__kernel void sap_gpu_ll_parallel(__global float* objects,__constant int* sizes,
                                  __global char* output,__global char* ramMemory,
                                  const unsigned int parallel_x, const unsigned int parallel_y,
                                  const unsigned int total_per_thread,unsigned int max_data_elemets)
{
    int index = get_global_id(0);
    int max_data=parallel_x*parallel_y;
    int begin_index = index*total_per_thread;
    int end_index = (index+1)*total_per_thread;
    if(begin_index >= max_data)
        return;
    int size_begin_real=0;
    for(int i=0;i<begin_index && i<max_data;++i)
    {
        size_begin_real+=sizes[i];
    }
    int realMemorySize = VAR1;
    int memorySize = VAR2;          //Memory Size
    struct RAM ram=RAM_create(&(ramMemory[index*realMemorySize]),memorySize);
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i]>1)
        {
            int n=sizes[i];
            int n_2=sizes[i]*2;
            __global float* dataX=CreateArray(&ram,objects,sizes[i],false,size_begin_real);
            __global unsigned short* iX=RadixSort(&ram,dataX,n_2);
            __global unsigned short* R=(__global unsigned short*)RAM_allocate(&ram,n_2*sizeof(unsigned short));
            for(unsigned short Ri=0;Ri<n_2;++Ri)
                R[iX[Ri]]=Ri;
            __global float* dataY=CreateArray(&ram,objects,sizes[i],true,size_begin_real);
            __global unsigned short* iY=RadixSort(&ram,dataY,n_2);
            ///Algorithm
            __global struct IntervalTree* S=IT_create(&ram);
            for(int i=0;i<n_2;++i)
            {
                unsigned short p=iY[i];
                if(p<n)
                {
                    struct Interval intervalo=Interval_create(R[p],R[p+n]);
                    __global struct List* result=IT_range_query(S,intervalo);
                    for(__global struct ListNode* d=result->root;d;d=d->next)
                    {
                        int i_a=dataY[p+n_2];
                        int i_b=dataY[(unsigned int)(iX[(unsigned int)(d->interval.data_left)]+n_2)];
                        output[get_id(i_a,i_b,max_data_elemets)]=1;
                    }
                    List_delete(result,&ram);
                    IT_insert(S,intervalo);
                }
                else
                {
                    IT_remove(S,Interval_create(R[p-n],R[p]));
                }
            }
            IT_delete(S,&ram);
            ///Algorithm
            ///Test
//            __global struct IntervalTree* itree=IT_create(&ram);
//            output[1]=IT_find(itree,Interval_create(3,1));
//            IT_insert(itree,Interval_create(2,1));
//            IT_insert(itree,Interval_create(3,1));
//            output[2]=IT_find(itree,Interval_create(2,1));
//            output[3]=IT_find(itree,Interval_create(3,1));
//            IT_remove(itree,Interval_create(3,1));
//            __global struct List* lista=IT_range_query(itree,Interval_create(4,0));
//            output[4]=lista->size;
//            List_delete(lista,&ram);
//            IT_delete(itree,&ram);
            ///Test
            RAM_free(&ram,(__global char*)iY,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(__global char*)dataY,sizes[i]*3*sizeof(float));
            RAM_free(&ram,(__global char*)R,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(__global char*)iX,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(__global char*)dataX,sizes[i]*3*sizeof(float));
        }
        size_begin_real+=sizes[i];
    }
}
