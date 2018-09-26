#ifndef SAP_LL_ITREE_PARALLEL_H_INCLUDED
#define SAP_LL_ITREE_PARALLEL_H_INCLUDED

#include "Object.h"

void* sap_ll_itree_thread_cpu(ALLEGRO_THREAD* thread,void* arg)
{
    DataParallelCPU* data = (DataParallelCPU*)arg;
//    std::stringstream& th_out=data->streamer;
//    th_out<<"ID: "<<data->thread_id<<std::endl;
//    th_out<<"Range: "<<data->begin_index<<" - "<<data->end_index<<std::endl;
    int max_data = data->max_x*data->max_y;
    if(data->begin_index >= max_data)
        return nullptr;
    //Begin
//    th_out<<"Checked Data"<<std::endl;
    for(int i=data->begin_index;i<data->end_index and i<max_data;++i)
    {
        int x=i%data->max_x;
        int y=i/data->max_x;
//        th_out<<"Data: ("<<x<<"-"<<y<<"): ";
//        th_out<<" - size: "<<data->objects[x][y].size()<<std::endl;
        std::vector<Object*>& objects=data->objects[x][y];
        int n=objects.size();
        int n_2=objects.size()*2;
        std::vector<MinMaxPoint> points(n_2);
        for(int i=0;i<n;++i)
        {
            points[i]=objects[i]->get_point(MinMaxType::T_MIN);
            points[i+n]=objects[i]->get_point(MinMaxType::T_MAX);
        }
        std::vector<int> R(n_2);
        std::vector<int> iX=LSDRS(points,compare_x_points);
        for(int i=0;i<n_2;++i)
            R[iX[i]]=i;
        std::vector<int> iY=LSDRS(points,compare_y_points);
        LL_DataStructure::IntervalTree S;
        for(int i=0;i<n_2;++i)
        {
            int p=iY[i];
            if(p<n)
            {
                LL_MathStructure::Interval intervalo(R[p],R[p+n]);
                auto result=S.range_query(intervalo);
                for(auto d=result.begin();d!=result.end();++d)
                {
                    int index_i=objects[p]->get_id();
                    int index_j=objects[iX[((*d)[0])]%n]->get_id();
                    if(index_i>index_j)
                        std::swap(index_i,index_j);
                    data->total_collision[index_i]+=1;
                    data->total_collision[index_j]+=1;
                    data->collision.push_back(std::pair<int,int>(index_i,index_j));
                }
                S.insert(intervalo);
            }
            else
            {
                LL_MathStructure::Interval intervalo(R[p-n],R[p]);
                S.remove(intervalo);
            }
        }
    }
    //END
    return nullptr;
}

void SAP_LL_ITREE_CPU_Parallel(std::vector<Object*>** objects,
                               std::vector<int>& total_collision,
                               std::list<std::pair<int,int>>& collision,
                               float* time_construction,float* time_collision,
                               int threads,float size_x,float size_y)
{
    LL::Chronometer chronometer;
    chronometer.play();
    //Construction BEGIN
    std::vector<DataParallelCPU*> data;
    std::list<ALLEGRO_THREAD*> thread_list;
    int data_by_thread=std::ceil(size_x*size_y/threads);
    for(int i=0;i<threads;++i)
    {
        data.push_back(new DataParallelCPU());
        DataParallelCPU& tdata=*data[i];
        tdata.objects=objects;
        tdata.total_collision=std::vector<int>(total_collision.size(),false);
        tdata.begin_index=i*data_by_thread;
        tdata.end_index=(i+1)*data_by_thread;
        tdata.max_x=size_x;
        tdata.max_y=size_y;
        tdata.thread_id=i;
        thread_list.push_back(al_create_thread(sap_ll_itree_thread_cpu, data[i]));
    }
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    for(ALLEGRO_THREAD* thread_i:thread_list)
        al_start_thread(thread_i);
    for(ALLEGRO_THREAD* thread_i:thread_list)
        al_join_thread(thread_i,nullptr);
//    std::cout<<"Thread Information"<<std::endl;
    for(int i=0;i<threads;++i)
    {
        DataParallelCPU* selected_data=data[i];
        for(unsigned int j=0;j<total_collision.size();++j)
            total_collision[j]+=selected_data->total_collision[j];
        collision.splice(collision.end(),selected_data->collision);
//        std::cout<<selected_data->streamer.str()<<std::endl;
        delete(selected_data);
    }
    //Collision END
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    for(ALLEGRO_THREAD* thread_i:thread_list)
        al_destroy_thread(thread_i);
}

#endif // SAP_LL_ITREE_PARALLEL_H_INCLUDED
