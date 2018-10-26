#include "Memory.h"

__kernel void sap_gpu_ll_parallel(__global float* objects,__constant int* sizes,
                                  __global char* output,//__local char* sub_coll,
                                  const unsigned int parallel_x, const unsigned int parallel_y,
                                  const unsigned int total_per_thread,unsigned int max_data_elemets)
{
    int index = get_global_id(0);
    int max_data=parallel_x*parallel_y;
    int begin_index = index*total_per_thread;
    int end_index = (index+1)*total_per_thread;
    if(index >= max_data)
        return;
    int size_begin_real=0;
    for(int i=0;i<begin_index && i<max_data;++i)
    {
        size_begin_real+=sizes[i];
    }
    char ramMemory[VAR1];           //RAM
    int memorySize = VAR2;          //Memory Size
    struct RAM ram=RAM_create(ramMemory,memorySize);
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i]>1)
        {
        }
        size_begin_real+=sizes[i];
    }
}
