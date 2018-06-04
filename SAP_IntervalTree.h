#ifndef SAP_INTERVALTREE_H_INCLUDED
#define SAP_INTERVALTREE_H_INCLUDED

#include "DataStructure/IntervalTree.h"

void SAP_IntervalTree(std::vector<Object>& objects,
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
    IntervalTree S;
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
            std::set<Interval> result=S.query(intervalo);
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

#endif // SAP_INTERVALTREE_H_INCLUDED
