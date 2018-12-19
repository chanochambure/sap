#include "stdio.h"

#include <LexRisLogic/Convert.h>

#include "GPURAM.h"

#include "GPU/RadixSort.h"
#include "Scene.h"
#include "GPU/Interval.h"
#include "GPU/IntervalTree.h"

/***GPU SAP***/
__device__ void sort(float* input,unsigned int output[], int begin, int size,bool x_y)
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

__device__ unsigned int get_id(unsigned int i,unsigned int j,unsigned int size)
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

__global__ void cuda_sap_gpu_parallel( float* objects,int* sizes,
									   char* output,//__local char* sub_coll,
									   const unsigned int parallel_x, const unsigned int parallel_y,
									   const unsigned int total_per_thread,unsigned int max_data_elemets,
									   const unsigned int local_size)
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
    unsigned int* ids =(unsigned int*) malloc(local_size*2*sizeof(unsigned int));   //local_size*2
    char* sub_coll = (char*) malloc((local_size*(local_size-1)/2)*sizeof(char));      //local_size*(local_size-1)/2
    char* in_use = (char*) malloc(local_size*sizeof(char));        //local_size
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i]>1)
        {
            int limit=sizes[i]*(sizes[i]-1)/2;
            for(int j=0;j<sizes[i];++j)
            {
                in_use[j]=0;
            }
            for(int j=0;j<limit;++j)
            {
                sub_coll[j]=0;
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
                            sub_coll[get_id(pi,pj,sizes[i])]=1;
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
                            if(sub_coll[get_id(pi,pj,sizes[i])])
                            {
                                int id_a=(pi+size_begin_real)*5;
                                int id_b=(pj+size_begin_real)*5;
                                int i_a=objects[id_a];
                                int i_b=objects[id_b];
                                output[get_id(i_a,i_b,max_data_elemets)]=1;
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
/***GPU SAP***/


/***GPU LL INTERVAL TREE SAP***/
__global__ void cuda_sap_ll_itree_gpu_parallel(float* objects,int* sizes,
											   char* output,//__local char* sub_coll,
											   const unsigned int parallel_x, const unsigned int parallel_y,
											   const unsigned int total_per_thread,unsigned int max_data_elemets,
											   const unsigned int local_size)
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
    for(int i=begin_index;i<end_index && i<max_data;++i)
    {
        if(sizes[i]>1)
        {
            int n=sizes[i];
            int n_2=sizes[i]*2;
            float* dataX=CreateArray(objects,sizes[i],false,size_begin_real);
            unsigned short* iX=RadixSort(dataX,n_2);
            free(dataX);
            unsigned short* R=(unsigned short*)malloc(n_2*sizeof(unsigned short));
            for(unsigned short Ri=0;Ri<n_2;++Ri)
                R[iX[Ri]]=Ri;
            float* dataY=CreateArray(objects,sizes[i],true,size_begin_real);
            unsigned short* iY=RadixSort(dataY,n_2);
            ///Algorithm
            IntervalTree S=IntervalTree_create();
            for(int i=0;i<n_2;++i)
            {
                unsigned short p=iY[i];
                if(p<n)
                {
                    struct CInterval intervalo=Interval_create(R[p],R[p+n]);
                    struct List result=IntervalTree_range_query(&S,intervalo);
                    for(struct ListNode* d=result.root;d;d=d->next)
                    {
                        int i_a=dataY[p+n_2];
                        int i_b=dataY[(unsigned int)(iX[(unsigned int)(d->interval.data[0])]+n_2)];
                        output[get_id(i_a,i_b,max_data_elemets)]=1;
                    }
                    List_clear(&result);
                    IntervalTree_insert(&S,intervalo);
                }
                else
                {
                    IntervalTree_remove(&S,Interval_create(R[p-n],R[p]));
                }
            }
            IntervalTree_clear(&S);
            free(iY);
            free(dataY);
            free(iX);
        }
        size_begin_real+=sizes[i];
    }
}
/***GPU LL INTERVAL TREE SAP***/

char* gpu_results=nullptr;
char* gpu_ram_results=nullptr;
float* input_buffer;
int* ref_buffer;
char* output_buffer;
char* ram_buffer;
size_t size_cuda_heap=256;
size_t size_ram=1*1024*1024;

void build_sap_gpu_parallel(int local_size,unsigned int total_objects,unsigned int total_real_objects, unsigned int size_x, unsigned int size_y)
{
    unsigned int max_outputs = (total_objects * (total_objects-1))/2;
    gpu_results=new char[max_outputs];
	cudaMalloc((void**)&input_buffer, total_real_objects * 5 * sizeof(float));
	cudaMalloc((void**)&ref_buffer, size_x*size_y * sizeof(int));
	cudaMalloc((void**)&output_buffer, max_outputs * sizeof(char));
}

void build_sap_ram_gpu_parallel(int local_size,unsigned int total_objects,unsigned int total_real_objects, unsigned int size_x, unsigned int size_y, unsigned int threads)
{
    unsigned int total_per_thread=std::ceil(1.0*size_x*size_y/threads);
    unsigned int max_outputs = (total_objects * (total_objects-1))/2;
    int threads_func=1.0*size_x*size_y/total_per_thread;
    gpu_ram_results=new char[max_outputs];
	cudaMalloc((void**)&input_buffer, total_real_objects * 5 * sizeof(float));
	cudaMalloc((void**)&ref_buffer, size_x*size_y * sizeof(int));
	cudaMalloc((void**)&output_buffer, max_outputs * sizeof(char));
	cudaMalloc((void**)&ram_buffer, threads_func * size_ram * sizeof(char));
}

bool SAP_GPU_Parallel(float* objects,
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
	cudaMemcpy( input_buffer, objects, total_real_objects * 5 * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy( ref_buffer, total_sizes, size_x*size_y * sizeof(int), cudaMemcpyHostToDevice);
    unsigned int total_per_thread=std::ceil(1.0*size_x*size_y/threads);
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    size_t local=1;
    size_t global=threads;
	cuda_sap_gpu_parallel<<<global,local>>>(input_buffer,ref_buffer,output_buffer,
											size_x,size_y,
											total_per_thread,total_objects,
											local_size);
	cudaThreadSynchronize();
    cudaError_t err;
    err = cudaGetLastError();
    if (err != cudaSuccess)
	{
        printf("Error: %s\n", cudaGetErrorString(err));
		return false;
	}
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    chronometer.play();
    cudaMemcpy( gpu_results, output_buffer, max_outputs*sizeof(char), cudaMemcpyDeviceToHost);
    unsigned int index_a=total_objects-1;
    unsigned int index_b=0;
    for(unsigned int i=0;i<max_outputs;++i)
    {
		// std::cout<<"Data "<<i<<": "<<gpu_results[i]<<std::endl;
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
    chronometer.stop();
    if(time_construction)
        *time_construction+=chronometer.get_time();
	return true;
}

bool SAP_GPU_LL_ITree_Parallel(float* objects,
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
	cudaMemcpy( input_buffer, objects, total_real_objects * 5 * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy( ref_buffer, total_sizes, size_x*size_y * sizeof(int), cudaMemcpyHostToDevice);
    unsigned int total_per_thread=std::ceil(1.0*size_x*size_y/threads);
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    size_t local=1;
    size_t global=threads;
	cuda_sap_ll_itree_gpu_parallel<<<global,local>>>(input_buffer,ref_buffer,output_buffer,
													 size_x,size_y,
													 total_per_thread,total_objects,
													 local_size);
	cudaThreadSynchronize();
    cudaError_t err;
    err = cudaGetLastError();
    if (err != cudaSuccess)
	{
        printf("Error: %s\n", cudaGetErrorString(err));
		return false;
	}
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    chronometer.play();
    cudaMemcpy( gpu_results, output_buffer, max_outputs*sizeof(char), cudaMemcpyDeviceToHost);
    unsigned int index_a=total_objects-1;
    unsigned int index_b=0;
    for(unsigned int i=0;i<max_outputs;++i)
    {
		// std::cout<<"Data "<<i<<": "<<int(gpu_results[i])<<std::endl;
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
    chronometer.stop();
    if(time_construction)
        *time_construction+=chronometer.get_time();
	return true;
}


unsigned int RAM_non_getTotalBytes(unsigned int ramBytes)
{
    return ramBytes+std::ceil(ramBytes/8.0);
}

bool SAP_GPU__RAM_LL_ITree_Parallel(float* objects,
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
        gpu_ram_results[i]=0;
	cudaMemcpy( input_buffer, objects, total_real_objects * 5 * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy( ref_buffer, total_sizes, size_x*size_y * sizeof(int), cudaMemcpyHostToDevice);
    unsigned int total_per_thread=std::ceil(1.0*size_x*size_y/threads);
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    size_t local=1;
    size_t global=threads;
	cuda_sap_ram_ll_itree_gpu_parallel<<<global,local>>>(input_buffer,ref_buffer,output_buffer,ram_buffer,
														 size_x,size_y,
														 total_per_thread,total_objects,
														 RAM_non_getTotalBytes(size_ram),size_ram);
	cudaThreadSynchronize();
    cudaError_t err;
    err = cudaGetLastError();
    if (err != cudaSuccess)
	{
        printf("Error: %s\n", cudaGetErrorString(err));
		return false;
	}
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    chronometer.play();
    cudaMemcpy( gpu_ram_results, output_buffer, max_outputs*sizeof(char), cudaMemcpyDeviceToHost);
    unsigned int index_a=total_objects-1;
    unsigned int index_b=0;
//    std::cout<<*((int*)(&(gpu_ram_results[0])))<<std::endl;
    for(unsigned int i=0;i<max_outputs;++i)
    {
        // std::cout<<"DATA "<<i<<": "<<int(gpu_ram_results[i])<<std::endl;
        if(gpu_ram_results[i])
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
    // std::cout<<std::endl;
    //Collision END
    chronometer.stop();
    if(time_construction)
        *time_construction+=chronometer.get_time();
	return true;
}

int main(int argc,char* argv[])
{
	// std::cout<<"Interval: "<<sizeof(CInterval)<<std::endl;
	// std::cout<<"ListNode: "<<sizeof(ListNode)<<std::endl;
	// std::cout<<"List: "<<sizeof(List)<<std::endl;
	// std::cout<<"QueueNode: "<<sizeof(QueueNode)<<std::endl;
	// std::cout<<"Queue: "<<sizeof(Queue)<<std::endl;
	// std::cout<<"StackNode: "<<sizeof(StackNode)<<std::endl;
	// std::cout<<"Stack: "<<sizeof(Stack)<<std::endl;
	// std::cout<<"IntervalTree: "<<sizeof(IntervalTree)<<std::endl;
	// std::cout<<"RAMInterval: "<<sizeof(Interval)<<std::endl;
	// std::cout<<"RAMListNode: "<<sizeof(RAMListNode)<<std::endl;
	// std::cout<<"RAMList: "<<sizeof(RAMList)<<std::endl;
	// std::cout<<"RAMQueueNode: "<<sizeof(RAMQueueNode)<<std::endl;
	// std::cout<<"RAMQueue: "<<sizeof(RAMQueue)<<std::endl;
	// std::cout<<"RAMStackNode: "<<sizeof(RAMStackNode)<<std::endl;
	// std::cout<<"RAMStack: "<<sizeof(RAMStack)<<std::endl;
	// std::cout<<"RAMIntervalTreeNode: "<<sizeof(RAMIntervalTreeNode)<<std::endl;
	// std::cout<<"RAMIntervalTree: "<<sizeof(RAMIntervalTree)<<std::endl;
    bool print_collision=false;
    bool collision=false;
    bool render_frames=false;
    bool print_frames=true;
    LL::random_generate_new_seed();
    bool create_new_map=false;
    int mision=0;
    int max_test=-1;
    bool command=false;
    bool command_2=false;
    bool unisize=false;
    int total=-1;
    int threads=1;
    int parallel_x=1;
    int parallel_y=1;
    bool autosave=true;
	size_cuda_heap=1024*1024*size_cuda_heap;
	cudaDeviceSetLimit(cudaLimitMallocHeapSize, size_cuda_heap);
	cudaDeviceGetLimit(&size_cuda_heap, cudaLimitMallocHeapSize);
	std::cout<<"Size Heap: "<<size_cuda_heap<<std::endl;
    if(argc >= 6)
    {
        create_new_map=LL::to_int(argv[1]);
        mision=LL::to_int(argv[2]);
        max_test=LL::to_int(argv[3]);
        collision=LL::to_int(argv[4]);
        autosave=LL::to_int(argv[5]);
        int counter=6;
        if(create_new_map)
        {
            if(argc>counter)
            {
                unisize=LL::to_int(argv[counter++]);
                total=LL::to_int(argv[counter++]);
                command=true;
            }
        }
        if(argc>counter)
        {
            parallel_x=LL::to_int(argv[counter++]);
            parallel_y=LL::to_int(argv[counter++]);
            threads=LL::to_int(argv[counter++]);
            command_2=true;
        }
    }
    else if(argc==1)
    {
        std::cout<<"Nuevo Mapa (1/0): ";
        std::cin>>create_new_map;
        std::cout<<"AutoSave (1/0): ";
        std::cin>>autosave;
    }
    else
    {
        std::cout<<"SaP <CREATE_NEW_MAP> <ALGORITHM> <TEST> <START> <AUTOSAVE> [<UNISIZE> <TOTAL>] ";
        std::cout<<"[<PARALLEL_X> <PARALLEL_Y> <THREADS>]"<<std::endl;
        return -1;
    }
    Scene scene("scene.txt");
    scene.load();
    if(create_new_map)
    {
        if(!command)
        {
            std::cout<<"Unisize (1/0): ";
            std::cin>>unisize;
        }
        while(total <= 0)
        {
            std::cout<<"Total de Elementos: ";
            std::cin>>total;
        }
        scene.create(unisize,total);
        scene.save();
    }
    if(max_test == 0)
        return 0;
    std::string name_function;
    bool (*collision_function_3)(
                               float*,
                               int*,
                               unsigned int,
                               std::vector<int>&,
                               std::list<std::pair<int,int>>&,
                               float*,
                               float*,
                               int,
                               unsigned int,
                               unsigned int,
                               unsigned int,
                               unsigned int
                               )=nullptr;
    while(1)
    {
        if(mision==11)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP GPU Parallel";
            collision_function_3=SAP_GPU_Parallel;
            scene.build_gpu(parallel_x,parallel_y);
            build_sap_gpu_parallel(scene.get_local_size(),scene.size(),scene.get_size_gpu_grid(),scene.size_x(),scene.size_y());
            break;
        }
        else if(mision==12)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP GPU LL ITREE Parallel";
            collision_function_3=SAP_GPU_LL_ITree_Parallel;
            scene.build_gpu(parallel_x,parallel_y);
            build_sap_gpu_parallel(scene.get_local_size(),scene.size(),scene.get_size_gpu_grid(),scene.size_x(),scene.size_y());
            break;
        }
        // else if(mision==13)
        // {
            // if(!command_2)
            // {
                // std::cout<<"Parallel X: ";
                // std::cin>>parallel_x;
                // std::cout<<"Parallel Y: ";
                // std::cin>>parallel_y;
                // std::cout<<"Threads: ";
                // std::cin>>threads;
            // }
            // name_function="SAP GPU LL ITREE Parallel using struct RAM";
            // collision_function_3=SAP_GPU__RAM_LL_ITree_Parallel;
            // scene.build_gpu(parallel_x,parallel_y);
            // build_sap_ram_gpu_parallel(scene.get_local_size(),scene.size(),scene.get_size_gpu_grid(),scene.size_x(),scene.size_y(),threads);
            // break;
        // }
        std::cout<<"Ingresar Funcion:\n";
        std::cout<<"11: SAP GPU Parallel\n";
		std::cout<<"12: SAP LL_IntervalTree GPU Parallel\n";
		// std::cout<<"13: SAP LL_IntervalTree RAM GPU Parallel\n";
        std::cout<<"Opcion:";
        std::cin>>mision;
    }
    std::vector<int> on_collision(scene.size(),false);
    LL_AL5::init_allegro();
    LL_AL5::primitives_addon();
    LL_AL5::text_addon();
    LL_AL5::Display display(LL_AL5::desktop_size_x,LL_AL5::desktop_size_y,SCENE_SIZE_X,SCENE_SIZE_Y);
    display.set_display_mode(ALLEGRO_FULLSCREEN_WINDOW);
    display.create();
    LL_AL5::Font font;
    font.set_path("comic.ttf");
    font.set_size(SCENE_SIZE_X/100.0);
    font.load_ttf_font();
    LL_AL5::Font another_font;
    another_font.set_path("comic.ttf");
    another_font.set_size(12.0);
    another_font.load_ttf_font();
    scene.set_font(&font);
    LL_AL5::KeyControl key_control;
    key_control.add_key("Polygon",ALLEGRO_KEY_P);
    key_control.add_key("Frames",ALLEGRO_KEY_F);
    key_control.add_key("Render",ALLEGRO_KEY_S);
    key_control.add_key("Collision",ALLEGRO_KEY_C);
    key_control.add_key("Print Collision",ALLEGRO_KEY_SPACE);
    key_control.add_key("Controls",ALLEGRO_KEY_L);
//    std::cout<<"Controls:"<<std::endl;
//    std::cout<<"Show/Hide Polygon: P"<<std::endl;
//    std::cout<<"Show/Hide Text: F"<<std::endl;
//    std::cout<<"Render Object: S"<<std::endl;
//    std::cout<<"Show Controls: L"<<std::endl;
//    std::cout<<"Make Collision: C"<<std::endl;
//    std::cout<<"Print Collision Info: SPACE"<<std::endl;
//    std::cout<<"--------------------"<<std::endl;
    int total_frames=0;
    LL::Chronometer time;
    std::list<std::pair<int,int>> collision_list;
    time.play();
    LL_AL5::Input input;
    input.register_display(display);
    input.keyboard_on();
    input.set_key_control(&key_control);
    LL_AL5::Color black;
    LL_AL5::Color green(0,255);
    LL_AL5::Color color;
    LL_AL5::Text fps_text;
    fps_text.set_font(&another_font);
    fps_text.set_color(black);
    fps_text.set_pos(10,10);
    fps_text="0 fps";
    LL_AL5::Text collision_text;
    collision_text.set_font(&another_font);
    collision_text.set_color(black);
    collision_text.set_pos(10,10+2*another_font.get_size());
    collision_text="0 s";
    LL_AL5::Text total_text;
    total_text.set_font(&another_font);
    total_text.set_color(black);
    total_text.set_pos(10,10+4*another_font.get_size());
    total_text=LL::to_string(scene.size());
    LL_AL5::Text max_time_text;
    max_time_text.set_font(&another_font);
    max_time_text.set_color(black);
    max_time_text.set_pos(10,10+9*another_font.get_size());
    max_time_text="0 s";
    LL_AL5::Text min_time_text;
    min_time_text.set_font(&another_font);
    min_time_text.set_color(black);
    min_time_text.set_pos(10,10+7*another_font.get_size());
    min_time_text="0 s";
    LL_AL5::Text prom_time_text;
    prom_time_text.set_font(&another_font);
    prom_time_text.set_color(black);
    prom_time_text.set_pos(10,10+11*another_font.get_size());
    prom_time_text="0 s";
    LL_AL5::Text total_test_text;
    total_test_text.set_font(&another_font);
    total_test_text.set_color(black);
    total_test_text.set_pos(10,10+14*another_font.get_size());
    total_test_text="0";
    double min_time=1000;
    double max_time=0;
    double acum=0;
    int test=0;
    std::list<std::pair<float,float>> tiempos;
    while(!input.get_display_status() && test!=max_test)
    {
        ++total_frames;
        display.clear();
        scene.draw_scene(&display);
        if(render_frames)
        {
            for(unsigned int i=0;i<scene.size();++i)
            {
                switch(on_collision[i])
                {
                    case 1:
                    {
                        color.red   = 0;
                        color.green = 0;
                        color.blue  = 255;
                        break;
                    }
                    case 0:
                    {
                        color.red   = 0;
                        color.green = 0;
                        color.blue  = 0;
                        break;
                    }
                    default:
                    {
                        color.red   =255;
                        color.green =0;
                        color.blue  =0;
                        break;
                    }
                }
                scene[i].draw_object(&display,green,color);
            }
            for(unsigned int i=0;i<scene.size();++i)
                scene[i].draw_text(&display,black);
        }
        if(print_frames)
        {
            display.draw(&fps_text,false);
            display.draw(&total_text,false);
            display.draw(&collision_text,false);
            display.draw(&min_time_text,false);
            display.draw(&max_time_text,false);
            display.draw(&prom_time_text,false);
            display.draw(&total_test_text,false);
        }
        display.refresh();
        if(input.get_event())
        {
            if(input["Collision"])
            {
                collision=!collision;
                input["Collision"]=false;
            }
            if(input["Print Collision"])
            {
                print_collision=!print_collision;
                input["Print Collision"]=false;
            }
            if(input["Polygon"])
            {
                draw_polygon=!draw_polygon;
                input["Polygon"]=false;
            }
            if(input["Controls"])
            {
                std::cout<<"Controls:"<<std::endl;
                std::cout<<"Show/Hide Polygon: P"<<std::endl;
                std::cout<<"Show/Hide Text: F"<<std::endl;
                std::cout<<"Render Object: S"<<std::endl;
                std::cout<<"Show Controls: L"<<std::endl;
                std::cout<<"Make Collision: C"<<std::endl;
                std::cout<<"Print Collision Info: SPACE"<<std::endl;
                std::cout<<"--------------------"<<std::endl;
                input["Controls"]=false;
            }
            if(input["Frames"])
            {
                print_frames=!print_frames;
                input["Frames"]=false;
            }
            if(input["Render"])
            {
                render_frames=!render_frames;
                input["Render"]=false;
            }
        }
        if(collision)
        {
            collision_list.clear();
            for(unsigned int i=0;i<scene.size();++i)
                on_collision[i]=0;
            float time_construction;
            float time_collision;
			bool data=false;
            if(collision_function_3)
            {
                data=(collision_function_3(scene.get_objects_gpu_grid(),
										   scene.get_sizes_gpu_grid(),
										   scene.size(),
										   on_collision,
										   collision_list,
										   &time_construction,
										   &time_collision,
										   threads,
										   scene.size_x(),
										   scene.size_y(),
										   scene.get_size_gpu_grid(),
										   scene.get_local_size()));
            }
			if(data)
			{
				float total_time=time_construction+time_collision;
				tiempos.push_back(std::pair<float,float>(time_construction,time_collision));
				total_test_text=LL::to_string(++test);
				collision_text=LL::to_string(total_time)+" s";
				acum+=total_time;
				if(min_time>total_time)
				{
					min_time=total_time;
					min_time_text=LL::to_string(min_time)+" s";
				}
				if(max_time<total_time)
				{
					max_time=total_time;
					max_time_text=LL::to_string(max_time)+" s";
				}
				prom_time_text=LL::to_string(acum/test)+" s";
			}
        }
        if(print_collision)
        {
            system("cls");
            std::cout<<"--------------------"<<std::endl;
            collision_list.sort();
            for(auto i=collision_list.begin();i!=collision_list.end();++i)
                std::cout<<"("<<(*i).first<<","<<(*i).second<<")"<<std::endl;
            std::cout<<"--------------------"<<std::endl;
            print_collision=false;
        }
        if(time.get_time()>1)
        {
            fps_text=LL::to_string(total_frames/time.get_time())+" fps";
            time.clear();
            total_frames=0;
        }
    }
    input.unregister_display();
    input.unregister_timer();
    if(test)
    {
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<"Total:     "<<scene.size()<<std::endl;
        std::cout<<"Test:      "<<test<<std::endl;
        std::cout<<"Algorithm: "<<name_function<<std::endl;
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<"Min:       "<<min_time<<std::endl;
        std::cout<<"Max:       "<<max_time<<std::endl;
        std::cout<<"Prom:      "<<acum/test<<std::endl;
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<std::endl;
    }
    if(autosave)
    {
        LL::FileStream txt_times;
        std::string path_name=name_function+" (S="+LL::to_string(scene.size())+").csv";
        txt_times.set_path(path_name);
        txt_times.load();
        txt_times.clear_file();
        txt_times.insert_line(0,tiempos.size());
        unsigned int index=0;
        for(auto tiempo:tiempos)
        {
            txt_times[index]=LL::to_string(tiempo.first)+";"+LL::to_string(tiempo.second)+";";
            std::replace(txt_times[index].begin(),txt_times[index].end(),'.',',');
            ++index;
        }
        std::cout<<"Saving: "<<path_name<<std::endl;
        txt_times.save();
    }
	if(gpu_results)
	{
		delete(gpu_results);
		cudaFree(input_buffer);
		cudaFree(ref_buffer);
		cudaFree(output_buffer);
	}
	if(gpu_ram_results)
	{
		delete(gpu_ram_results);
		cudaFree(input_buffer);
		cudaFree(ref_buffer);
		cudaFree(output_buffer);
		cudaFree(ram_buffer);
	}
    return 0;
}
