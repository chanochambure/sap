#ifndef SAP_RSTARTREE1D_H_INCLUDED
#define SAP_RSTARTREE1D_H_INCLUDED


typedef RStarTree<Interval, 1, 2, 5> RStarTree_SAP;

RStarTree_SAP::BoundingBox object_to_star_mbb_1D(Interval interval)
{
    RStarTree_SAP::BoundingBox mbb;
	mbb.edges[0].first  = interval.first;
	mbb.edges[0].second = interval.second;
    return mbb;
}

struct IntervalVisitor
{
	int count;
	bool ContinueVisiting;
	std::list<Interval> result;

	IntervalVisitor() : count(0), ContinueVisiting(true) {};

	void operator()(const RStarTree_SAP::Leaf* const leaf)
	{
	    result.push_back(leaf->leaf);
	}
};


void SAP_RStarTree1D(std::vector<Object>& objects,
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
    RStarTree_SAP S;
    IntervalVisitor visitor;
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
            visitor=S.Query(RStarTree_SAP::AcceptOverlapping(object_to_star_mbb_1D(intervalo)),IntervalVisitor());
            for(auto d=visitor.result.begin();d!=visitor.result.end();++d)
            {
                int index_i=p;
                int index_j=iX[(d->first)]%n;
                if(index_i>index_j)
                    std::swap(index_i,index_j);
                ++total_collision[index_i];
                ++total_collision[index_j];
                collision.push_back(std::pair<int,int>(index_i,index_j));
            }
            S.Insert(intervalo, object_to_star_mbb_1D(intervalo));
        }
        else
        {
            int mini=R[p-n];
            int maxi=R[p];
            if(mini>maxi)
                std::swap(mini,maxi);
            Interval intervalo(mini,maxi);
            S.RemoveItem(intervalo);
        }
    }
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
}

#endif // SAP_RSTARTREE1D_H_INCLUDED
