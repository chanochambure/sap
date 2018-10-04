void sort(__constant float* input,unsigned int output[], int begin, int size,bool x_y)
{
    int axes=(x_y)?3:1;
    int last=0;
    for(int i=0;i<size;++i)
    {
        for(int a=0;a<2;++a)
        {
            int asignacion=0;
            int real_id=(begin+i)*5+a+axes;
            for(int j=0;j<last;++j)
            {
                int real=output[j];
                if(input[real]>input[real_id])
                {
                    break;
                }
                ++asignacion;
            }
            for(int d=last;d>asignacion;--d)
            {
                output[d]=output[d-1];
            }
            output[asignacion]=real_id;
            ++last;
        }
    }
}

__kernel void sap_gpu_parallel(__constant float* objects,__constant int* sizes,__global char* output,
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
    unsigned int ids[VAR1] = {0};   //local_size*2
    char sub_coll[VAR2] = {0};      //local_size*local_size
    char in_use[VAR3] = {0};        //local_size
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        int size_begin_real=0;
        for(int pp=0;pp<i;++pp)
            size_begin_real+=sizes[pp];
        if(sizes[i])
        {
            for(int j=0;j<sizes[i];++j)
            {
                in_use[j]=0;
                for(int k=0;k<sizes[i];++k)
                    sub_coll[j*sizes[i]+k]=0;
            }
            sort(objects,ids,size_begin_real,sizes[i],false);
            for(int j=0;j<sizes[i]*2;++j)
            {
                int real_id=ids[j];
                if(real_id%5==1)
                {
                    for(int l=0;l<sizes[i];++l)
                    {
                        if(in_use[l])
                        {
                            int pi=real_id/5-size_begin_real;
                            int pj=l;
                            sub_coll[pi*sizes[i]+pj]=1;
                        }
                    }
                    in_use[real_id/5-size_begin_real]=1;
                }
                else if(real_id%5==2)
                {
                    in_use[real_id/5-size_begin_real]=0;
                }
            }
            for(int j=0;j<sizes[i];++j)
                in_use[j]=0;
            sort(objects,ids,size_begin_real,sizes[i],true);
            for(int j=0;j<sizes[i]*2;++j)
            {
                int real_id=ids[j];
                if(real_id%5==3)
                {
                    for(int l=0;l<sizes[i];++l)
                    {
                        if(in_use[l])
                        {
                            int pi=real_id/5-size_begin_real;
                            int pj=l;
                            if(sub_coll[pi*sizes[i]+pj] || sub_coll[pj*sizes[i]+pi])
                            {
                                int id_a=(pi+size_begin_real)*5;
                                int i_a=objects[id_a];
                                int id_b=(pj+size_begin_real)*5;
                                int i_b=objects[id_b];
                                output[max_data_elemets*i_a+i_b]=1;
                            }
                        }
                    }
                    in_use[real_id/5-size_begin_real]=1;
                }
                else if(real_id%5==4)
                {
                    in_use[real_id/5-size_begin_real]=0;
                }
            }
        }
        size_begin_real+=sizes[i];
    }
}
