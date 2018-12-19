#ifndef RADIXSORT_H_INCLUDED
#define RADIXSORT_H_INCLUDED

#include "Memory.h"
#include "Queue.h"

unsigned int* RadixSort(struct RAM* ram,float* points,unsigned int size)
{
    unsigned int* index=(unsigned int*)RAM_allocate(ram,size*sizeof(unsigned int));
    for(unsigned int i=0;i<size;++i)
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
    struct Queue* queues=(struct Queue*)RAM_allocate(ram,10*sizeof(struct Queue));
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
    RAM_free(ram,(char*)queues,10*sizeof(struct Queue));
    return index;
}


#endif // RADIXSORT_H_INCLUDED
