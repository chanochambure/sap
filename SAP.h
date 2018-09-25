#ifndef SAP_H_INCLUDED
#define SAP_H_INCLUDED

#include "Object.h"

void SAP(std::vector<Object>& objects,
         std::vector<int>& total_collision,
         std::list<std::pair<int,int>>& collision,
         float* time_construction,float* time_collision)
{
    LL::Chronometer chronometer;
    //First Axis
    chronometer.play();
    std::list<IndexMinMaxPoint> points;
    for(unsigned int i=0;i<objects.size();++i)
    {
        points.push_back(IndexMinMaxPoint(objects[i].get_point(MinMaxType::T_MIN),i));
        points.push_back(IndexMinMaxPoint(objects[i].get_point(MinMaxType::T_MAX),i));
    }
    points.sort(index_compare_x_points);
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    //Collision
    chronometer.play();
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
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
    //Second Axis
    chronometer.play();
    points.sort(index_compare_y_points);
    chronometer.stop();
    if(time_construction)
        *time_construction+=chronometer.get_time();
    //Collision
    chronometer.play();
    indexes.clear();
    for(IndexMinMaxPoint& point:points)
    {
        if(point.point.type == MinMaxType::T_MIN)
        {
            for(unsigned int index:indexes)
            {
                if(answer_x.find(LL_MathStructure::Interval(point.index,index)) != answer_x.end())
                {
                    ++total_collision[point.index];
                    ++total_collision[index];
                    collision.push_back(std::pair<int,int>(point.index,index));
                }
            }
            indexes.insert(point.index);
        }
        else
            indexes.erase(point.index);
    }
    chronometer.stop();
    if(time_collision)
        *time_collision+=chronometer.get_time();
}

#endif // SAP_H_INCLUDED
