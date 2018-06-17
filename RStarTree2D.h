#ifndef RSTARTREE2D_H_INCLUDED
#define RSTARTREE2D_H_INCLUDED

#include "RStarTree/RStarTree.h"

#include "Object.h"

typedef RStarTree<Object*, 2, 2, 5> RStarTree_S;

RStarTree_S::BoundingBox object_to_star_mbb(Object& object)
{
    LL_MathStructure::Point ini=object.get_point(MinMaxType::T_MIN).point;
    LL_MathStructure::Point fin=object.get_point(MinMaxType::T_MAX).point;
    RStarTree_S::BoundingBox mbb;
	mbb.edges[0].first  = ini[0];
	mbb.edges[0].second = fin[0];
	mbb.edges[1].first  = ini[1];
	mbb.edges[1].second = fin[1];
    return mbb;
}

struct Visitor2D
{
	int count;
	bool ContinueVisiting;
	std::list<Object*> result;

	Visitor2D() : count(0), ContinueVisiting(true) {};

	void operator()(const RStarTree_S::Leaf* const leaf)
	{
	    result.push_back(leaf->leaf);
	}
};

void RStarTree2D(std::vector<Object>& objects,
                 std::vector<int>& total_collision,
                 std::list<std::pair<int,int>>& collision,
                 float* time_construction,float* time_collision)
{
    Visitor2D visitor;
    LL::Chronometer chronometer;
    chronometer.play();
    RStarTree_S S;
    for(unsigned int i=0;i<objects.size();++i)
        S.Insert(&objects[i], object_to_star_mbb(objects[i]));
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    for(unsigned int i=0;i<objects.size();++i)
    {
        S.RemoveItem(&objects[i]);
        visitor=S.Query(RStarTree_S::AcceptOverlapping(object_to_star_mbb(objects[i])),Visitor2D());
        for(auto d=visitor.result.begin();d!=visitor.result.end();++d)
        {
            int index_i=(*d)->get_id();
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
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
}

#endif // RSTARTREE2D_H_INCLUDED
