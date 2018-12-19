#ifndef RADIXSORT_H_INCLUDED
#define RADIXSORT_H_INCLUDED

#include "Queue.h"

__device__ float* CreateArray(float* numbers,unsigned int n,bool x_y,int begin)
{
    int axis=(x_y)?3:1;
    float* return_value=(float*)malloc(n*3*sizeof(float));
    for(unsigned int i=0;i<n;++i)
    {
        return_value[i+(2*n)]=numbers[(begin+i)*5];
        return_value[i]=numbers[(begin+i)*5+axis];
        return_value[i+n]=numbers[(begin+i)*5+axis+1];
    }
    return return_value;
}

__device__ unsigned short* RadixSort(float* points,unsigned int size)
{
    unsigned short* index=(unsigned short*)malloc(size*sizeof(unsigned short));
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
    struct Queue* queues=(struct Queue*)malloc(10*sizeof(struct Queue));
    for(unsigned int i=0;i<10;++i)
        queues[i]=Queue_create();
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
    free(queues);
    return index;
}


#endif // RADIXSORT_H_INCLUDED
