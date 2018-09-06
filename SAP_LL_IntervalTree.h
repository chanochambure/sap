#ifndef SAP_LL_INTERVALTREE_H_INCLUDED
#define SAP_LL_INTERVALTREE_H_INCLUDED

#include <LexRisLogic/DataStructures/IntervalTree.h>

void SAP_LL_IntervalTree(std::vector<Object>& objects,
                         std::vector<int>& total_collision,
                         std::list<std::pair<int,int>>& collision,
                         float* time_construction,float* time_collision)
{
    LL::Chronometer chronometer;
    chronometer.play();
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
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
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
                int index_i=p;
                int index_j=iX[((*d)[0])]%n;
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
            LL_MathStructure::Interval intervalo(R[p-n],R[p]);
            S.remove(intervalo);
        }
    }
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
}

#endif // SAP_LL_INTERVALTREE_H_INCLUDED
