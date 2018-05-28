#ifndef RTREE1D_H_INCLUDED
#define RTREE1D_H_INCLUDED

#include <LexRisLogic/DataStructures/RTree.h>

#include "Object.h"

LL_MathStructure::MBB pair_to_mbb(Interval interval)
{
    LL_MathStructure::MBB mbb;
    mbb.set_dimension(1);
    mbb.first_point[0]=interval.first;
    mbb.second_point[0]=interval.second;
    return mbb;
}

void SAP_RTree1D(std::vector<Object>& objects,
                 std::vector<int>& total_collision,
                 std::list<std::pair<int,int>>& collision)
{
    int n=objects.size();
    int n_2=objects.size()*2;
    std::vector<MinMaxPoint> points(n_2);
    for(int i=0;i<n;++i)
    {
        points[i]=objects[i].get_point(MinMaxType::T_MIN);
        points[i+n]=objects[i].get_point(MinMaxType::T_MAX);
    }
    std::vector<int> R(n_2);
    std::vector<int> iX=LSDRS(points,compare_x_points);
    for(int i=0;i<n_2;++i)
        R[iX[i]]=i;
    std::vector<int> iY=LSDRS(points,compare_y_points);
    LL_DataStructure::RTree<Interval,1,5> S(pair_to_mbb);
    for(int i=0;i<n_2;++i)
    {
        int p=iY[i];
        if(p<n)
        {
            int mini=R[p];
            int maxi=R[p+n];
            if(mini>maxi)
                std::swap(mini,maxi);
            Interval intervalo(mini,maxi);
            std::list<Interval> result=S.range_query(pair_to_mbb(intervalo));
            for(auto d=result.begin();d!=result.end();++d)
            {
                int index_i=p;
                int index_j=iX[(d->first)]%n;
                if(index_i>index_j)
                    std::swap(index_i,index_j);
                ++total_collision[index_i];
                ++total_collision[index_j];
                collision.push_back(std::pair<int,int>(index_i,index_j));
            }
            S.insert(intervalo);
        }
        else
        {
            int mini=R[p-n];
            int maxi=R[p];
            if(mini>maxi)
                std::swap(mini,maxi);
            Interval intervalo(mini,maxi);
            S.remove(intervalo);
        }
    }
}

#endif // RTREE1D_H_INCLUDED
