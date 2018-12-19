///MEMORY

struct RAM
{
    char* space;
    unsigned int startByte;
    unsigned int ramBytes;
    unsigned int totalBytes;
};

__device__ unsigned int RAM_getTotalBytes(unsigned int ramBytes)
{
    return ramBytes+ceil(ramBytes/8.0);
}

__device__ struct RAM RAM_create(char* space,unsigned int ramBytes)
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

__device__ char* RAM_getDataByIndex(struct RAM* ram,unsigned int index)
{
    return &(ram->space[ram->startByte+index]);
}

__device__ unsigned int RAM_getIndexByData(struct RAM* ram,char* data)
{
    return (unsigned int)(data-ram->space)-ram->startByte;
}

__device__ char* RAM_allocate(struct RAM* ram,unsigned int bytes)
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

__device__ void RAM_free(struct RAM* ram,char* data,unsigned int bytes)
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
struct RAMQueueNode
{
    struct RAMQueueNode* next;
    unsigned int data;
};

struct RAMQueue
{
    struct RAMQueueNode* root;
    unsigned int size;
    struct RAM* ram;
};

__device__ struct RAMQueue RAM_Queue_create(struct RAM* ram)
{
    struct RAMQueue new_queue;
    new_queue.root=NULL;
    new_queue.size=0;
    new_queue.ram=ram;
    return new_queue;
}

__device__ void RAM_Queue_push(struct RAMQueue* queue,unsigned int data)
{
    struct RAMQueueNode* new_node=(struct RAMQueueNode*)RAM_allocate(queue->ram,sizeof(struct RAMQueueNode));
    new_node->data=data;
    new_node->next=NULL;
    struct RAMQueueNode** last=&(queue->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    queue->size+=1;
}

__device__ void RAM_Queue_pop(struct RAMQueue* queue)
{
    if(queue->size)
    {
        struct RAMQueueNode* first=queue->root;
        queue->root=queue->root->next;
        RAM_free(queue->ram,(char*)first,sizeof(struct RAMQueueNode));
        queue->size-=1;
    }
}

__device__ unsigned int RAM_Queue_front(struct RAMQueue* queue)
{
    if(queue->size)
        return queue->root->data;
    return 0;
}
///QUEUE

///RADIX SORT
__device__ float* RAM_CreateArray(struct RAM* ram,float* numbers,unsigned int n,bool x_y,int begin)
{
    int axis=(x_y)?3:1;
    float* return_value=(float*)RAM_allocate(ram,n*3*sizeof(float));
    for(unsigned int i=0;i<n;++i)
    {
        return_value[i+(2*n)]=numbers[(begin+i)*5];
        return_value[i]=numbers[(begin+i)*5+axis];
        return_value[i+n]=numbers[(begin+i)*5+axis+1];
    }
    return return_value;
}

__device__ unsigned short* RAM_RadixSort(struct RAM* ram,float* points,unsigned short size)
{
    unsigned short* index=(unsigned short*)RAM_allocate(ram,size*sizeof(unsigned short));
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
    struct RAMQueue* queues=(struct RAMQueue*)RAM_allocate(ram,10*sizeof(struct RAMQueue));
    for(unsigned int i=0;i<10;++i)
        queues[i]=RAM_Queue_create(ram);
    for(unsigned int i=0;i<cifras;i++)
    {
    	for(unsigned int j=0;j<size;++j)
    	{
    		int d = (int)(points[index[j]]/pow((float)10,(float)i))%10;
    		RAM_Queue_push(queues+d,index[j]);
    	}
    	int p=0;
    	for(unsigned int j=0;j<10;++j)
    	{
    		while(queues[j].size)
    		{
    			index[p]=RAM_Queue_front(&(queues[j]));
    			RAM_Queue_pop(queues+j);
    			++p;
    		}
    	}
    }
    RAM_free(ram,(char*)queues,10*sizeof(struct RAMQueue));
    return index;
}
///RADIX SORT

///INTERVAL
struct Interval
{
    float data_left;
    float data_right;
};

__device__ struct Interval RAM_Interval_create(float left,float right)
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
__device__ float RAM_Interval_get_mid_point(struct Interval interval)
{
    return (interval.data_left+interval.data_right)/2.0;
}
__device__ bool RAM_Interval_contains_p(struct Interval interval,float point)
{
    return interval.data_left<=point && point<=interval.data_right;
}
__device__ bool RAM_Interval_contains(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left<=another_interval.data_left && another_interval.data_right<=interval.data_right;
}
__device__ bool RAM_Interval_intersects(struct Interval interval,struct Interval another_interval)
{
    float ini_point=max(interval.data_left,another_interval.data_left);
    float end_point=min(interval.data_right,another_interval.data_right);
    return ini_point<=end_point;
}
__device__ bool RAM_Interval_is_left_of_p(struct Interval interval,float point)
{
    return interval.data_right<point;
}
__device__ bool RAM_Interval_is_right_of_p(struct Interval interval,float point)
{
    return point<interval.data_left;
}
__device__ bool RAM_Interval_is_left_of(struct Interval interval,struct Interval another_interval)
{
    return RAM_Interval_is_left_of_p(interval,another_interval.data_left);
}
__device__ bool RAM_Interval_is_right_of(struct Interval interval,struct Interval another_interval)
{
    return RAM_Interval_is_right_of_p(interval,another_interval.data_right);
}
__device__ bool RAM_Interval_equality(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left==another_interval.data_left && interval.data_right==another_interval.data_right;
}
__device__ bool RAM_Interval_difference(struct Interval interval,struct Interval another_interval)
{
    return interval.data_left!=another_interval.data_left || interval.data_right!=another_interval.data_right;
}
///INTERVAL

///LIST
__device__ bool RAM_List_comp_increasing(struct Interval interval,struct Interval another_interval)
{
    if(interval.data_left==another_interval.data_left)
        return interval.data_right<another_interval.data_right;
    return interval.data_left<another_interval.data_left;
}

__device__ bool RAM_List_comp_decreasing(struct Interval interval,struct Interval another_interval)
{
    if(interval.data_right==another_interval.data_right)
        return interval.data_left>another_interval.data_left;
    return interval.data_right>another_interval.data_right;
}

struct RAMListNode
{
    struct RAMListNode* next;
    struct Interval interval;
};

struct RAMList
{
    struct RAMListNode* root;
    unsigned int size;
    struct RAM* ram;
    bool sortType;
};

__device__ struct RAMList RAM_List_create(struct RAM* ram,bool sortType)
{
    struct RAMList new_list;
    new_list.root=NULL;
    new_list.size=0;
    new_list.ram=ram;
    new_list.sortType=sortType;
    return new_list;
}

__device__ void RAM_List_clear(struct RAMList* list)
{
    struct RAMListNode* node=list->root;
    while(node)
    {
        struct RAMListNode* temp=node->next;
        node->next=NULL;
        RAM_free(list->ram,(char*)node,sizeof(struct RAMListNode));
        node=temp;
    }
    list->root=NULL;
    list->size=0;
}

__device__ void RAM_List_copy(struct RAMList* to,struct RAMList* from)
{
    RAM_List_clear(to);
    struct RAMListNode** node=&(to->root);
    for(struct RAMListNode* i=from->root;i;i=i->next)
    {
        *node=(struct RAMListNode*)RAM_allocate(to->ram,sizeof(struct RAMListNode));
        (*node)->interval=i->interval;
        node=&((*node)->next);
    }
    to->size=from->size;
}

__device__ bool __RAM_List_private_find(struct Interval interval,struct RAMListNode*** node,bool sortType)
{
    // Decreasing
    if(sortType)
    {
        for(;*(*node) && RAM_List_comp_decreasing((*(*node))->interval,interval);*node=&((*(*node))->next));
        return (*(*node) && RAM_Interval_equality((*(*node))->interval,interval));
    }
    // Increasing
    else
    {
        for(;*(*node) && RAM_List_comp_increasing((*(*node))->interval,interval);*node=&((*(*node))->next));
        return (*(*node) && RAM_Interval_equality((*(*node))->interval,interval));
    }
}

__device__ bool RAM_List_find(struct RAMList* list,struct Interval interval)
{
    struct RAMListNode** node=&list->root;
    return __RAM_List_private_find(interval,&node,list->sortType);
}

__device__ bool RAM_List_insert(struct RAMList* list,struct Interval interval)
{
    struct RAMListNode** position_node=&list->root;
    if(__RAM_List_private_find(interval,&position_node,list->sortType))
        return false;
    struct RAMListNode* new_node=(struct RAMListNode*)RAM_allocate(list->ram,sizeof(struct RAMListNode));
    new_node->interval=interval;
    new_node->next=*position_node;
    *position_node=new_node;
    list->size+=1;
    return true;
}

__device__ bool RAM_List_remove(struct RAMList* list,struct Interval interval)
{
    struct RAMListNode** position_node=&list->root;
    if(__RAM_List_private_find(interval,&position_node,list->sortType))
    {
        list->size-=1;
        struct RAMListNode* temp=(*position_node)->next;
        (*position_node)->next=NULL;
        RAM_free(list->ram,(char*)(*position_node),sizeof(struct RAMListNode));
        *position_node=temp;
        return true;
    }
    return false;
}

///LIST

///INTERVALTREE

struct RAMIntervalTreeNode
{
    struct RAMList increasing;
    struct RAMList decreasing;
    struct RAMIntervalTreeNode* left;
    struct RAMIntervalTreeNode* right;
    float mid_point;
    int height;
    struct RAM* ram;
};

__device__ struct RAMIntervalTreeNode* ITN_create_empty(struct RAM* ram)
{
    struct RAMIntervalTreeNode* node=(struct RAMIntervalTreeNode*)RAM_allocate(ram,sizeof(struct RAMIntervalTreeNode));
    node->increasing=RAM_List_create(ram,false);
    node->decreasing=RAM_List_create(ram,true);
    node->mid_point=0;
    node->height=0;
    node->left=NULL;
    node->right=NULL;
    node->ram=ram;
    return node;
}
__device__ struct RAMIntervalTreeNode* ITN_create(struct Interval interval,struct RAM* ram)
{
    struct RAMIntervalTreeNode* node=(struct RAMIntervalTreeNode*)RAM_allocate(ram,sizeof(struct RAMIntervalTreeNode));
    node->increasing=RAM_List_create(ram,false);
    node->decreasing=RAM_List_create(ram,true);
    RAM_List_insert(&(node->increasing),interval);
    RAM_List_insert(&(node->decreasing),interval);
    node->mid_point=RAM_Interval_get_mid_point(interval);
    node->height=1;
    node->left=NULL;
    node->right=NULL;
    node->ram=ram;
    return node;
}
__device__ int ITN_get_left_height(struct RAMIntervalTreeNode* node)
{
    if(node->left)
        return node->left->height;
    return 0;
}
__device__ int ITN_get_right_height(struct RAMIntervalTreeNode* node)
{
    if(node->right)
        return node->right->height;
    return 0;
}
__device__ void ITN_update_height(struct RAMIntervalTreeNode* node)
{
    node->height=max(ITN_get_left_height(node),
                     ITN_get_right_height(node))+1;
}
__device__ void ITN_destroy_node(struct RAMIntervalTreeNode* node)
{
    if(node->left)
        ITN_destroy_node(node->left);
    if(node->right)
        ITN_destroy_node(node->right);
    RAM_List_clear(&(node->increasing));
    RAM_List_clear(&(node->decreasing));
    node->left=NULL;
    node->right=NULL;
    RAM_free(node->ram,(char*)node,sizeof(struct RAMIntervalTreeNode));
}

///STACK
struct RAMStackNode
{
    struct RAMStackNode* next;
    struct RAMIntervalTreeNode* data;
};

struct RAMStack
{
    struct RAMStackNode* root;
    unsigned int size;
    struct RAM* ram;
};

__device__ struct RAMStack RAM_Stack_create(struct RAM* ram)
{
    struct RAMStack new_stack;
    new_stack.root=NULL;
    new_stack.size=0;
    new_stack.ram=ram;
    return new_stack;
}

__device__ void RAM_Stack_push(struct RAMStack* stack,struct RAMIntervalTreeNode* data)
{
    struct RAMStackNode* new_node=(struct RAMStackNode*)RAM_allocate(stack->ram,sizeof(struct RAMStackNode));
    new_node->data=data;
    new_node->next=NULL;
    struct RAMStackNode** last=&(stack->root);
    for(;*last;last=&((*last)->next));
    *last=new_node;
    stack->size+=1;
}

__device__ void RAM_Stack_pop(struct RAMStack* stack)
{
    if(stack->size)
    {
        struct RAMStackNode** last=&(stack->root);
        for(;(*last)->next;last=&((*last)->next));
        RAM_free(stack->ram,(char*)(*last),sizeof(struct RAMStackNode));
        *last=NULL;
        stack->size-=1;
    }
}

__device__ struct RAMIntervalTreeNode* RAM_Stack_top(struct RAMStack* stack)
{
    if(stack->size)
    {
        struct RAMStackNode* last=stack->root;
        for(;last->next;last=last->next);
        return last->data;
    }
    return 0;
}
///STACK

struct RAMIntervalTree
{
    struct RAMIntervalTreeNode* root;
    struct RAM* ram;
};

__device__ void IT_clear(struct RAMIntervalTree* itree)
{
    if(itree->root)
        ITN_destroy_node(itree->root);
    itree->root=NULL;
}

__device__ struct RAMIntervalTree IT_create(struct RAM* ram)
{
    struct RAMIntervalTree itree;
    itree.root=NULL;
    itree.ram=ram;
    return itree;
}

__device__ struct RAMIntervalTreeNode* __IT_pr_get_overlaps(struct RAMIntervalTreeNode* to,struct RAMIntervalTreeNode* from);

__device__ struct RAMIntervalTreeNode* __IT_pr_balance(struct RAMIntervalTreeNode* node)
{
    int status=ITN_get_left_height(node)-ITN_get_right_height(node);
    if(status<-1)
    {
        if(ITN_get_left_height(node->right)>ITN_get_right_height(node->right))
        {
            struct RAMIntervalTreeNode* current_node=node->right;
            struct RAMIntervalTreeNode* head=current_node->left;
            current_node->left=head->right;
            head->right=current_node;
            ITN_update_height(current_node);
            head->right=__IT_pr_get_overlaps(head,current_node);
            node->right=head;
        }
        struct RAMIntervalTreeNode* head=node->right;
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
            struct RAMIntervalTreeNode* current_node=node->left;
            struct RAMIntervalTreeNode* head=current_node->right;
            current_node->right=head->left;
            head->left=current_node;
            ITN_update_height(current_node);
            head->left=__IT_pr_get_overlaps(head,current_node);
            node->left=head;
        }
        struct RAMIntervalTreeNode* head=node->left;
        node->left=head->right;
        head->right=node;
        ITN_update_height(node);
        head->right=__IT_pr_get_overlaps(head,node);
        return head;
    }
    return node;
}

__device__ struct RAMIntervalTreeNode* __IT_pr_get_overlaps(struct RAMIntervalTreeNode* to,struct RAMIntervalTreeNode* from)
{
    struct RAMList temp_list;
    if(to->mid_point<from->mid_point)
    {
        temp_list=RAM_List_create(to->ram,false);
        for(struct RAMListNode* i=from->increasing.root;i;i=i->next)
        {
            struct Interval next=i->interval;
            if(RAM_Interval_is_right_of_p(next,to->mid_point))
                break;
            RAM_List_insert(&temp_list,next);
        }
    }
    else
    {
        temp_list=RAM_List_create(to->ram,true);
        for(struct RAMListNode* i=from->decreasing.root;i;i=i->next)
        {
            struct Interval next=i->interval;
            if(RAM_Interval_is_left_of_p(next,to->mid_point))
                break;
            RAM_List_insert(&temp_list,next);
        }
    }
    for(struct RAMListNode* i=temp_list.root;i;i=i->next)
    {
        struct Interval next=i->interval;
        RAM_List_remove(&(from->increasing),next);
        RAM_List_remove(&(from->decreasing),next);
    }
    for(struct RAMListNode* i=temp_list.root;i;i=i->next)
    {
        RAM_List_insert(&(to->increasing),i->interval);
        RAM_List_insert(&(to->decreasing),i->interval);
    }
    RAM_List_clear(&temp_list);
    if(!from->increasing.size)
    {
        struct RAMIntervalTreeNode* target=from;
        if(!(target->left) && !(target->right))
        {
            ITN_destroy_node(target);
            return NULL;
        }
        if(!(target->left))
        {
            struct RAMIntervalTreeNode* new_root=target->right;
            target->right=NULL;
            ITN_destroy_node(target);
            return new_root;
        }
        struct RAMIntervalTreeNode* node=target->left;
        struct RAMStack stack=RAM_Stack_create(target->ram);
        while(node->right)
        {
            RAM_Stack_push(&stack,node);
            node=node->right;
        }
        if(stack.size)
        {
            struct RAMIntervalTreeNode* top_node=RAM_Stack_top(&stack);
            top_node->right=node->left;
            node->left=target->left;
        }
        node->right=target->right;
        struct RAMIntervalTreeNode* new_root=node;
        while(stack.size)
        {
            node=RAM_Stack_top(&stack);
            RAM_Stack_pop(&stack);
            if(stack.size)
            {
                struct RAMIntervalTreeNode* top_node=RAM_Stack_top(&stack);
                top_node->right=__IT_pr_get_overlaps(new_root,node);
            }
            else
                new_root->left=__IT_pr_get_overlaps(new_root,node);
        }
        target->left=NULL;
        target->right=NULL;
        ITN_destroy_node(target);
        return __IT_pr_balance(new_root);
    }
    return from;
}

__device__ bool __IT_pr_find(struct RAMIntervalTreeNode* node,struct Interval interval)
{
    if(node)
    {
        if(RAM_Interval_contains_p(interval,node->mid_point))
        {
            if(RAM_List_find(&(node->increasing),interval))
                return true;
        }
        else if(RAM_Interval_is_left_of_p(interval,node->mid_point))
            return __IT_pr_find(node->left,interval);
        else
            return __IT_pr_find(node->right,interval);
    }
    return false;
}

__device__ struct RAMIntervalTreeNode* __IT_pr_insert(struct RAMIntervalTree* itree,struct RAMIntervalTreeNode* node,struct Interval interval)
{
    if(node==NULL)
    {
        struct RAMIntervalTreeNode* new_node=ITN_create(interval,itree->ram);
        return new_node;
    }
    if(RAM_Interval_contains_p(interval,node->mid_point))
    {
        if(RAM_List_insert(&(node->decreasing),interval))
        {
            RAM_List_insert(&(node->increasing),interval);
        }
        return node;
    }
    else if(RAM_Interval_is_left_of_p(interval,node->mid_point))
        node->left=__IT_pr_insert(itree,node->left,interval);
    else
        node->right=__IT_pr_insert(itree,node->right,interval);
    ITN_update_height(node);
    return __IT_pr_balance(node);
}

__device__ struct RAMIntervalTreeNode* __IT_pr_remove(struct RAMIntervalTree* itree,struct RAMIntervalTreeNode* node,struct Interval interval)
{
    if(node==NULL)
        return NULL;
    if(RAM_Interval_contains_p(interval,node->mid_point))
    {
        if(RAM_List_remove(&(node->decreasing),interval))
        {
            RAM_List_remove(&(node->increasing),interval);
            if(node->increasing.size==0)
            {
                struct RAMIntervalTreeNode* target=node;
                if(!(target->left) && !(target->right))
                {
                    ITN_destroy_node(target);
                    return NULL;
                }
                if(!(target->left))
                {
                    struct RAMIntervalTreeNode* new_root=target->right;
                    target->right=NULL;
                    ITN_destroy_node(target);
                    return new_root;
                }
                struct RAMIntervalTreeNode* extra_node=target->left;
                struct RAMStack stack=RAM_Stack_create(target->ram);
                while(extra_node->right)
                {
                    RAM_Stack_push(&stack,extra_node);
                    extra_node=extra_node->right;
                }
                if(stack.size)
                {
                    struct RAMIntervalTreeNode* top_node=RAM_Stack_top(&stack);
                    top_node->right=extra_node->left;
                    extra_node->left=target->left;
                }
                extra_node->right=target->right;
                struct RAMIntervalTreeNode* new_root=extra_node;
                while(stack.size)
                {
                    extra_node=RAM_Stack_top(&stack);
                    RAM_Stack_pop(&stack);
                    if(stack.size)
                    {
                        struct RAMIntervalTreeNode* top_node=RAM_Stack_top(&stack);
                        top_node->right=__IT_pr_get_overlaps(new_root,extra_node);
                    }
                    else
                        new_root->left=__IT_pr_get_overlaps(new_root,extra_node);
                }
                target->left=NULL;
                target->right=NULL;
                ITN_destroy_node(target);
                return __IT_pr_balance(new_root);
            }
        }
    }
    else if(RAM_Interval_is_left_of_p(interval,node->mid_point))
        node->left=__IT_pr_remove(itree,node->left,interval);
    else
        node->right=__IT_pr_remove(itree,node->right,interval);
    ITN_update_height(node);
    return __IT_pr_balance(node);
}

__device__ void __IT_pr_range_query(struct RAMList* data,struct RAMIntervalTreeNode* node,struct Interval query)
{
    while(node)
    {
        if(RAM_Interval_contains_p(query,node->mid_point))
        {
            for(struct RAMListNode* i=node->increasing.root;i;i=i->next)
                RAM_List_insert(data,i->interval);
            __IT_pr_range_query(data,node->left,query);
            __IT_pr_range_query(data,node->right,query);
            break;
        }
        else if(RAM_Interval_is_left_of_p(query,node->mid_point))
        {
            for(struct RAMListNode* i=node->increasing.root;i;i=i->next)
            {
                if(!RAM_Interval_intersects(query,i->interval))
                    break;
                RAM_List_insert(data,i->interval);
            }
            node=node->left;
        }
        else if(RAM_Interval_is_right_of_p(query,node->mid_point))
        {
            for(struct RAMListNode* i=node->decreasing.root;i;i=i->next)
            {
                if(!RAM_Interval_intersects(query,i->interval))
                    break;
                RAM_List_insert(data,i->interval);
            }
            node=node->right;
        }
    }
}

__device__ bool IT_find(struct RAMIntervalTree* itree,struct Interval interval)
{
    return __IT_pr_find(itree->root,interval);
}
__device__ void IT_insert(struct RAMIntervalTree* itree,struct Interval interval)
{
    itree->root=__IT_pr_insert(itree,itree->root,interval);
}
__device__ void IT_remove(struct RAMIntervalTree* itree,struct Interval interval)
{
    if(itree->root)
        itree->root=__IT_pr_remove(itree,itree->root,interval);
}
__device__ struct RAMList IT_range_query(struct RAMIntervalTree* itree, Interval query)
{
    struct RAMList data=RAM_List_create(itree->ram,false);
    __IT_pr_range_query(&data,itree->root,query);
    return data;
}

///INTERVALTREE

__device__ unsigned int ram_get_id(unsigned int i,unsigned int j,unsigned int size)
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
__global__ void cuda_sap_ram_ll_itree_gpu_parallel(float* objects, int* sizes,
												char* output, char* ramMemory,
												const unsigned int parallel_x, const unsigned int parallel_y,
												const unsigned int total_per_thread,unsigned int max_data_elemets,
												int realMemorySize, int memorySize)
{
	int xindex = threadIdx.x + blockIdx.x * blockDim.x;
	int yindex = threadIdx.y + blockIdx.y * blockDim.y;
	int index = xindex + (gridDim.x * gridDim.y * yindex);
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
    struct RAM ram=RAM_create(&(ramMemory[index*realMemorySize]),memorySize);
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i]>1)
        {
            int n=sizes[i];
            int n_2=sizes[i]*2;
            float* dataX=RAM_CreateArray(&ram,objects,sizes[i],false,size_begin_real);
            unsigned short* iX=RAM_RadixSort(&ram,dataX,n_2);
            unsigned short* R=(unsigned short*)RAM_allocate(&ram,n_2*sizeof(unsigned short));
            for(unsigned short Ri=0;Ri<n_2;++Ri)
                R[iX[Ri]]=Ri;
            float* dataY=RAM_CreateArray(&ram,objects,sizes[i],true,size_begin_real);
            unsigned short* iY=RAM_RadixSort(&ram,dataY,n_2);
            ///Algorithm
            struct RAMIntervalTree S=IT_create(&ram);
            for(int new_i=0;new_i<n_2;++new_i)
            {
                unsigned short p=iY[new_i];
                if(p<n)
                {
                    struct Interval intervalo=RAM_Interval_create(R[p],R[p+n]);
                    struct RAMList result=IT_range_query(&S,intervalo);
                    for(struct RAMListNode* d=result.root;d;d=d->next)
                    {
                        int i_a=dataY[p+n_2];
                        int i_b=dataY[(unsigned int)(iX[(unsigned int)(d->interval.data_left)]+n_2)];
                        output[ram_get_id(i_a,i_b,max_data_elemets)]=1;
                    }
                    RAM_List_clear(&result);
                    IT_insert(&S,intervalo);
                }
                else
                {
                    IT_remove(&S,RAM_Interval_create(R[p-n],R[p]));
                }
            }
            IT_clear(&S);
            ///Algorithm
            RAM_free(&ram,(char*)iY,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(char*)dataY,sizes[i]*3*sizeof(float));
            RAM_free(&ram,(char*)R,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(char*)iX,sizes[i]*2*sizeof(unsigned short));
            RAM_free(&ram,(char*)dataX,sizes[i]*3*sizeof(float));
        }
        size_begin_real+=sizes[i];
    }
}
