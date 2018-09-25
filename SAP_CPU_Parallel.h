#ifndef SAP_CPU_PARALLEL_H_INCLUDED
#define SAP_CPU_PARALLEL_H_INCLUDED

#include "Object.h"

void* sap_thread_cpu(ALLEGRO_THREAD* thread,void* arg)
{
    DataParallelCPU* data = (DataParallelCPU*)arg;
    int max_data = data->max_x*data->max_y;
    if(data->begin_index >= max_data)
        return nullptr;
    //Begin
    std::list<IndexMinMaxPoint> points;
    for(int i=data->begin_index;i<data->end_index and i<max_data;++i)
    {
        int x=i%data->max_x;
        int y=i/data->max_x;
        for(Object* object:data->objects[x][y])
        {
            points.push_back(IndexMinMaxPoint(object->get_point(MinMaxType::T_MIN),object->get_id()));
            points.push_back(IndexMinMaxPoint(object->get_point(MinMaxType::T_MAX),object->get_id()));
        }
    }
    points.sort(index_compare_x_points);
    std::set<unsigned int> indexes;
    std::set<LL_MathStructure::Interval> answer_x;
    for(IndexMinMaxPoint& point:points)
    {
        if(point.point.type == MinMaxType::T_MIN)
        {
            for(unsigned int index:indexes)
                answer_x.insert(LL_MathStructure::Interval(point.index,index));
            indexes.insert(point.index);
        }
        else
            indexes.erase(point.index);
    }
    points.sort(index_compare_y_points);
    indexes.clear();
    for(IndexMinMaxPoint& point:points)
    {
        if(point.point.type == MinMaxType::T_MIN)
        {
            for(unsigned int index:indexes)
            {
                if(answer_x.find(LL_MathStructure::Interval(point.index,index)) != answer_x.end())
                {
                    data->total_collision[point.index]+=1;
                    data->total_collision[index]+=1;
                    data->collision.push_back(std::pair<int,int>(point.index,index));
                }
            }
            indexes.insert(point.index);
        }
        else
            indexes.erase(point.index);
    }
    //END
    return nullptr;
}

void SAP_CPU_Parallel(std::list<Object*>** objects,
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
        thread_list.push_back(al_create_thread(sap_thread_cpu, data[i]));
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
    for(int i=0;i<threads;++i)
    {
        DataParallelCPU* selected_data=data[i];
        for(unsigned int j=0;j<total_collision.size();++j)
            total_collision[j]+=selected_data->total_collision[j];
        collision.splice(collision.end(),selected_data->collision);
        delete(selected_data);
    }
    //Collision END
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    for(ALLEGRO_THREAD* thread_i:thread_list)
        al_destroy_thread(thread_i);
}

#endif // SAP_CPU_PARALLEL_H_INCLUDED
