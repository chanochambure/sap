void sort_x(__global float* input,__local unsigned int* output, int begin, int size)
{
    int last=0;
    for(int i=0;i<size;++i)
    {
        for(int a=0;a<2;++a)
        {
            int asignacion=0;
            int real_id=(begin+i)*5+a+1;
            for(int j=0;j<last;++j)
            {
                int real_other_id=output[j];
                if(input[real_other_id]>input[real_id])
                    break;
                ++asignacion;
            }
            for(int j=last;j>asignacion;--j)
                output[j]=output[j-1];
            output[asignacion]=real_id;
            ++last;
        }
    }
}

void sort_y(__global float* input,__local unsigned int* output, int begin, int size)
{
    int last=0;
    for(int i=0;i<size;++i)
    {
        for(int a=0;a<2;++a)
        {
            int asignacion=0;
            int real_id=(begin+i)*5+a+3;
            for(int j=0;j<last;++j)
            {
                int real_other_id=output[j];
                if(input[real_other_id]>input[real_id])
                    break;
                ++asignacion;
            }
            for(int j=last;j>asignacion;--j)
                output[j]=output[j-1];
            output[asignacion]=real_id;
            ++last;
        }
    }
}

__kernel void sap_gpu_parallel(__global float* objects,__global int* sizes,
                              __global char* output, __local unsigned int* ids,
                              __local char* sub_coll,__local char* in_use,
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
        size_begin_real+=sizes[i];
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i])
        {
            for(int j=0;j<sizes[i];++j)
                in_use[j]=0;
            for(int j=0;j<sizes[i];++j)
            {
                sub_coll[j*2+0]=0;
                sub_coll[j*2+1]=0;
            }
            sort_x(objects,ids,size_begin_real,sizes[i]);
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
                            if(pi<pj)
                                sub_coll[pi*sizes[i]+pj]=1;
                            else
                                sub_coll[pj*sizes[i]+pi]=1;
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
            sort_y(objects,ids,size_begin_real,sizes[i]);
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
                            if(pi<pj)
                            {
                                if(sub_coll[pi*sizes[i]+pj])
                                {
                                    int id_a=(pi+size_begin_real)*5;
                                    int i_a=objects[id_a];
                                    int id_b=(pj+size_begin_real)*5;
                                    int i_b=objects[id_b];
                                    output[max_data_elemets*i_a+i_b]=1;
                                }
                            }
                            else
                            {
                                if(sub_coll[pj*sizes[i]+pi])
                                {
                                    int id_a=(pj+size_begin_real)*5;
                                    int i_a=objects[id_a];
                                    int id_b=(pi+size_begin_real)*5;
                                    int i_b=objects[id_b];
                                    output[max_data_elemets*i_a+i_b]=1;
                                }
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
