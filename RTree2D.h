#ifndef RTREE2D_H_INCLUDED
#define RTREE2D_H_INCLUDED

#include <LexRisLogic/DataStructures/RTree.h>

#include "Object.h"

LL_MathStructure::MBB object_to_mbb(Object object)
{
    LL_MathStructure::MBB mbb;
    mbb.set_dimension(2);
    mbb.first_point=object.get_point(MinMaxType::T_MIN).point;
    mbb.second_point=object.get_point(MinMaxType::T_MAX).point;
    return mbb;
}

void RTree2D(std::vector<Object>& objects,
             std::vector<int>& total_collision,
             std::list<std::pair<int,int>>& collision)
{
    LL_DataStructure::RTree<Object,2,5> S(object_to_mbb);
    for(unsigned int i=0;i<objects.size();++i)
        S.insert(objects[i]);
    for(unsigned int i=0;i<objects.size();++i)
    {
        S.remove(objects[i]);
        std::list<Object> result=S.range_query(object_to_mbb(objects[i]));
        for(auto d=result.begin();d!=result.end();++d)
        {
            int index_i=d->get_id();
            int index_j=objects[i].get_id();
            if(index_i>index_j)
                std::swap(index_i,index_j);
            if(index_i!=index_j)
            {
                ++total_collision[index_i];
                ++total_collision[index_j];
                collision.push_back(std::pair<int,int>(index_i,index_j));
            }
        }
    }
}

#endif // RTREE2D_H_INCLUDED
