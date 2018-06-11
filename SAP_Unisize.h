#ifndef SAP_UNISIZE_H_INCLUDED
#define SAP_UNISIZE_H_INCLUDED

#include "Object.h"

void SAP_Unisize_Box(std::vector<Object>& objects,
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
    std::map<int,int> S;
    for(int i=0;i<n_2;++i)
    {
        int p=iY[i];
        if(p<n)
        {
            S[R[p]]=1;
            S[R[p+n]]=1;
            int mini=R[p];
            int maxi=R[p+n];
            if(mini>maxi)
                std::swap(mini,maxi);
            std::map<int,int>::iterator result_f=S.upper_bound(mini);
            for(std::map<int,int>::iterator i=result_f;i!=S.end() && i->first<maxi;++i)
            {
                int index_i=p%n;
                int index_j=iX[(i->first)]%n;
                ++total_collision[index_i];
                ++total_collision[index_j];
                if(index_i>index_j)
                    std::swap(index_i,index_j);
                collision.push_back(std::pair<int,int>(index_i,index_j));
            }
        }
        else
        {
            auto rpn=S.find(R[p-n]);
            auto rp=S.find(R[p]);
            if(rpn!=S.end())
                S.erase(rpn);
            if(rp!=S.end())
                S.erase(rp);
        }
    }
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
}

#endif // SAP_UNISIZE_H_INCLUDED
