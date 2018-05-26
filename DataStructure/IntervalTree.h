#ifndef INCLUDED_INTERVAL_TREE_H
#define INCLUDED_INTERVAL_TREE_H

#include <vector>

struct Interval
{
    float first;
    float second;
    Interval()
    {
        first=0;
        second=0;
    }
    Interval(float x, float y)
    {
        first=x;
        second=y;
    }
};

void print(Interval interval)
{
    std::cout<<"["<<interval.first<<", ";
    std::cout<<interval.second<<"] ";
}

void print(std::vector<Interval> intervals)
{
    for(auto i=intervals.begin();i != intervals.end();++i)
        print(*i);
    std::cout<<std::endl;
}

bool operator == (Interval i,Interval j)
{
    return i.first == j.first and i.second == j.second;
}

float compareBegin(Interval i,Interval j)
{
    float d=(i.first-j.first);
    if(d)
        return d;
    return (i.second-j.second);
}

float compareEnd(Interval i,Interval j)
{
    float d=(i.second-j.second);
    if(d)
        return d;
    return (i.first-j.first);
}

class IntervalTree
{
    private:
        struct _S_Structure_Node
        {
            float mid;
            _S_Structure_Node* left;
            _S_Structure_Node* right;
            std::vector<Interval> left_points;
            std::vector<Interval> right_points;
            unsigned int count=0;
            //Constructors
            _S_Structure_Node(float mid, _S_Structure_Node* left, _S_Structure_Node* right,
                          std::vector<Interval> left_points, std::vector<Interval> right_points)
            {
                this->mid=mid;
                this->left=left;
                this->right=right;
                this->left_points=left_points;
                this->right_points=right_points;
                this->count=this->left_points.size();
                if(this->left)
                    this->count += left->count;
                if(this->right)
                    this->count += right->count;
            }
            _S_Structure_Node* create_interval_tree(std::vector<Interval> intervals)
            {
                if(intervals.size() == 0)
                    return nullptr;
                std::vector<float> pts;
                for(auto i=intervals.begin();i!=intervals.end();++i)
                {
                    pts.push_back(i->first);
                    pts.push_back(i->second);
                }
                std::sort(pts.begin(),pts.end());
                float mid = pts[pts.size()>>1];
                std::vector<Interval> leftIntervals;
                std::vector<Interval> rightIntervals;
                std::vector<Interval> centerIntervals;
                for(unsigned int i=0; i<intervals.size(); ++i)
                {
                    Interval s = intervals[i];
                    if(s.second < mid)
                        leftIntervals.push_back(s);
                    else if(mid < s.first)
                        rightIntervals.push_back(s);
                    else
                        centerIntervals.push_back(s);
                }
                //Split center intervals
                std::vector<Interval> leftPoints = centerIntervals;
                std::vector<Interval> rightPoints = centerIntervals;
                std::sort(leftPoints.begin(),leftPoints.end(),compareBegin);
                std::sort(rightPoints.begin(),rightPoints.end(),compareEnd);
                return new _S_Structure_Node(mid,
                                             create_interval_tree(leftIntervals),
                                             create_interval_tree(rightIntervals),
                                             leftPoints,
                                             rightPoints);
            }
            void rebuild(std::vector<Interval> intervals)
            {
                _S_Structure_Node* ntree = create_interval_tree(intervals);
                if(ntree)
                {
                    this->mid = ntree->mid;
                    this->left = ntree->left;
                    this->right = ntree->right;
                    this->left_points = ntree->left_points;
                    this->right_points = ntree->right_points;
                    this->count = ntree->count;
                    delete(ntree);
                }
            }
            void rebuild_with_interval(Interval interval)
            {
                std::vector<Interval> result;
                get_intervals(result);
                result.push_back(interval);
                rebuild(result);
            }
            //Other
            void get_intervals(std::vector<Interval>& result)
            {
                result.insert(result.end(), this->left_points.begin(), this->left_points.end());
                if(this->left)
                    this->left->get_intervals(result);
                if(this->right)
                    this->right->get_intervals(result);
            }
            //Functions
            void insert(Interval interval)
            {
                std::cout<<interval.first<<" "<<interval.second<<std::endl;
                unsigned int weight = this->count - this->left_points.size();
                this->count+=1;
                if(interval.second < this->mid)
                {
                    if(this->left)
                    {
                        if(4*(this->left->count+1) > 3*(weight+1))
                            rebuild_with_interval(interval);
                        else
                            this->left->insert(interval);
                    }
                    else
                        this->left=create_interval_tree({interval});
                }
                else if(interval.first > this->mid)
                {
                    if(this->right)
                    {
                        if(4*(this->right->count+1) > 3*(weight+1))
                            rebuild_with_interval(interval);
                        else
                            this->right->insert(interval);
                    }
                    else
                        this->right=create_interval_tree({interval});
                }
                else
                {
                    auto l=std::lower_bound(this->left_points.begin(),this->left_points.end(),interval,compareBegin);
                    auto r=std::lower_bound(this->right_points.begin(),this->right_points.end(),interval,compareEnd);
                    this->left_points.insert(l,interval);
                    this->right_points.insert(r,interval);
                }
            }
        };
        unsigned int _V_size=0;
        _S_Structure_Node* _V_root=nullptr;
    public:
        IntervalTree()
        {
        }
        void insert(Interval interval)
        {
            if(_V_root)
                _V_root->insert(interval);
            else
                _V_root = new _S_Structure_Node(interval.first, nullptr, nullptr, {interval}, {interval});
        }
        std::vector<Interval> get_intervals()
        {
            std::vector<Interval> result;
            _V_root->get_intervals(result);
            return result;
        }
};

#endif // INCLUDED_INTERVAL_TREE_H
