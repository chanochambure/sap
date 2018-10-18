#ifndef SAP_GPU_PARALLEL_H_INCLUDED
#define SAP_GPU_PARALLEL_H_INCLUDED

#include <CL/cl.h>
#include <LexRisLogic/FileStream.h>

std::string sap_gpu_kernel_program;
//OPENCL
cl_platform_id sap_gpu_platform;
cl_device_id sap_gpu_device_id;
cl_context sap_gpu_context;
cl_command_queue sap_gpu_commands;
cl_program sap_gpu_program;
cl_kernel sap_gpu_kernel;

char* gpu_results=nullptr;

void build_sap_gpu_parallel(int local_size,unsigned int total_objects)
{
    unsigned int max_outputs = (total_objects * (total_objects-1))/2;
    //LOAD FILE .c
    LL::FileStream file;
    file.set_path("SAP_GPU_Parallel.c");
    if(file.load())
    {
        for(unsigned int i=0;i<file.size();++i)
            sap_gpu_kernel_program+=file[i]+"\n";
        replace(sap_gpu_kernel_program,"VAR1",LL::to_string(local_size*2));
        replace(sap_gpu_kernel_program,"VAR2",LL::to_string(local_size*(local_size-1)/2));
        replace(sap_gpu_kernel_program,"VAR3",LL::to_string(local_size));
    }
    //GPU CONF
    int err;
    const char* kernel_c_str=sap_gpu_kernel_program.c_str();
    cl_platform_id sap_gpu_platform_temp[2];
    cl_uint num;
    err = clGetPlatformIDs(2, sap_gpu_platform_temp, &num);
    if(err < 0)
    {
        printf("Couldn't identify a platform");
        exit(1);
    }
    sap_gpu_platform=sap_gpu_platform_temp[num-1];
    //GET GPU DEVICE
    err = clGetDeviceIDs(sap_gpu_platform, CL_DEVICE_TYPE_GPU, 1, &sap_gpu_device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Couldn't access any devices");
        exit(1);
    }
    char buf[4096];
    err = clGetDeviceInfo(sap_gpu_device_id, CL_DEVICE_NAME, sizeof(buf), buf, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Couldn't get device info");
        exit(1);
    }
    printf("Device Name : %s\n", buf);
    sap_gpu_context = clCreateContext(0, 1, &sap_gpu_device_id, NULL, NULL, &err);
    if (!sap_gpu_context)
    {
        printf("Error: Failed to create a compute context!\n");
        exit(1);
    }
    sap_gpu_commands = clCreateCommandQueue(sap_gpu_context, sap_gpu_device_id, 0, &err);
    if (!sap_gpu_commands)
    {
        printf("Error: Failed to create a command commands!\n");
        exit(1);
    }
    sap_gpu_program = clCreateProgramWithSource(sap_gpu_context, 1, &kernel_c_str, NULL, &err);
    if (!sap_gpu_program)
    {
        printf("Error: Failed to create compute program!\n");
        exit(1);
    }
    err = clBuildProgram(sap_gpu_program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(sap_gpu_program, sap_gpu_device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }
    sap_gpu_kernel = clCreateKernel(sap_gpu_program, "sap_gpu_parallel", &err);
    if (!sap_gpu_kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }
    gpu_results=new char[max_outputs];
}

void delete_sap_gpu_parallel()
{
    if(sap_gpu_kernel_program.size())
    {
        clReleaseProgram(sap_gpu_program);
        clReleaseKernel(sap_gpu_kernel);
        clReleaseCommandQueue(sap_gpu_commands);
        clReleaseContext(sap_gpu_context);
    }
    if(gpu_results)
        delete[](gpu_results);
    gpu_results=nullptr;
}

void SAP_GPU_Parallel(float* objects,
                      int* total_sizes,
                      unsigned int total_objects,
                      std::vector<int>& total_collision,
                      std::list<std::pair<int,int>>& collision,
                      float* time_construction,float* time_collision,
                      int threads,unsigned int size_x,unsigned int size_y,
                      unsigned int total_real_objects,
                      unsigned int local_size)
{
    LL::Chronometer chronometer;
    chronometer.play();
    //Construction BEGIN
    unsigned int max_outputs = (total_objects * (total_objects-1))/2;
    for(unsigned int i=0;i<max_outputs;++i)
        gpu_results[i]=0;
    int err;
    cl_mem input_buffer;
    cl_mem ref_buffer;
    cl_mem output_buffer;
    input_buffer = clCreateBuffer(sap_gpu_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, total_real_objects * 5 * sizeof(float), objects, &err);
    if(err < 0)
    {
        printf("Couldn't create input buffer");
        return;
    }
    ref_buffer = clCreateBuffer(sap_gpu_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size_x*size_y * sizeof(int), total_sizes, &err);
    if(err < 0)
    {
        printf("Couldn't create reference buffer");
        return;
    }
    output_buffer= clCreateBuffer(sap_gpu_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , max_outputs * sizeof(char), gpu_results, &err);
    if(err < 0)
    {
        printf("Couldn't create output buffer");
        return;
    }
    unsigned int total_per_thread=std::ceil(1.0*size_x*size_y/threads);
    err = clSetKernelArg(sap_gpu_kernel, 0, sizeof(cl_mem), &input_buffer);
    err |= clSetKernelArg(sap_gpu_kernel, 1, sizeof(cl_mem), &ref_buffer);
    err |= clSetKernelArg(sap_gpu_kernel, 2, sizeof(cl_mem), &output_buffer);
//    err |= clSetKernelArg(sap_gpu_kernel, 3, (local_size*(local_size-1)/2) * sizeof(char), NULL);
    err |= clSetKernelArg(sap_gpu_kernel, 3, sizeof(unsigned int), &size_x);
    err |= clSetKernelArg(sap_gpu_kernel, 4, sizeof(unsigned int), &size_y);
    err |= clSetKernelArg(sap_gpu_kernel, 5, sizeof(unsigned int), &total_per_thread);
    err |= clSetKernelArg(sap_gpu_kernel, 6, sizeof(unsigned int), &total_objects);
    if(err < 0)
    {
        printf("Couldn't create a kernel argument");
        return;
    }
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    size_t local=1;
    size_t global=threads;
    err = clEnqueueNDRangeKernel(sap_gpu_commands, sap_gpu_kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        std::cout<<err<<std::endl;
        return;
    }
    clFinish(sap_gpu_commands);
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    chronometer.play();
    err = clEnqueueReadBuffer(sap_gpu_commands,output_buffer,CL_TRUE,0,sizeof(char)*max_outputs,gpu_results,0,NULL,NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        return;
    }
    unsigned int index_a=total_objects-1;
    unsigned int index_b=0;
    for(unsigned int i=0;i<max_outputs;++i)
    {
        if(gpu_results[i])
        {
            total_collision[index_a]+=1;
            total_collision[index_b]+=1;
            collision.push_back(std::pair<int,int>(index_a,index_b));
        }
        ++index_b;
        if(index_a==index_b)
        {
            --index_a;
            index_b=0;
        }
    }
    //Collision END
    clReleaseMemObject(input_buffer);
    clReleaseMemObject(output_buffer);
    chronometer.stop();
    if(time_construction)
        *time_construction+=chronometer.get_time();
}

#endif // SAP_GPU_PARALLEL_H_INCLUDED
