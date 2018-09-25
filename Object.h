#ifndef INCLUDED_OBJECT_H
#define INCLUDED_OBJECT_H

#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <queue>
#include <algorithm>

#include <LexRisLogic/Allegro5/Allegro5.h>
#include <LexRisLogic/Allegro5/Display.h>
#include <LexRisLogic/Allegro5/Input.h>
#include <LexRisLogic/Allegro5/Color.h>
#include <LexRisLogic/Allegro5/Text.h>
#include <LexRisLogic/Allegro5/Timer.h>
#include <LexRisLogic/Allegro5/Primitives.h>

#include <LexRisLogic/MathStructures/Point.h>
#include <LexRisLogic/MathStructures/Polygon.h>
#include <LexRisLogic/MathStructures/MBB.h>

#include <LexRisLogic/Math.h>
#include <LexRisLogic/Convert.h>
#include <LexRisLogic/Time.h>

bool draw_polygon=true;

enum MinMaxType
{
    T_MIN = 0,
    T_MAX = 1
};

struct MinMaxPoint
{
    MinMaxType type=MinMaxType::T_MIN;
    LL_MathStructure::Point point;
};

int compareTo(float a,float b)
{
    if(a<b)
        return -1;
    else if(a>b)
        return 1;
    return 0;
}

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
    float getMidpoint()
    {
		return (first+second)/2.0;
    }
	bool contains(float point)
	{
	    return first<=point && point<second;
	}
	bool isLeftOf(float point)
	{
	    return second <= point;
	}
	bool isRightOf(float point){
		return point < first;
	}
	bool intersects(Interval query)
	{
	    return LL::segment_collision(first,second,query.first,query.second);
	}
	bool contains(Interval query)
	{
	    return contains(query.first) and contains(query.second);
	}
	bool isLeftOf(Interval other){
		return isLeftOf(other.first);
	}
	bool isRightOf(Interval other){
		return isRightOf(other.second);
	}
};

bool operator == (Interval i,Interval j)
{
    return i.first == j.first and i.second == j.second;
}

bool operator < (Interval i,Interval j)
{
    if(i.first == j.first)
        return i.second < j.second;
    return i.first < j.first;
}

bool compare_x_points(const MinMaxPoint& first, const MinMaxPoint& second)
{
    return first.point[0]<second.point[0];
}

bool compare_y_points(const MinMaxPoint& first, const MinMaxPoint& second)
{
    return first.point[1]<second.point[1];
}

std::vector<int> LSDRS(std::vector<MinMaxPoint>& points,bool(*compare_func)(const MinMaxPoint&,const MinMaxPoint&))
{
    std::vector<int> index(points.size());
    std::size_t n(0);
    std::generate(std::begin(index),std::end(index),[&]{ return n++;});
    MinMaxPoint max_value=points[0];
    std::vector<std::queue<unsigned int>> queues(10);
    for(unsigned int i=1;i<points.size();++i)
    	if(compare_func(max_value,points[i]))
    		max_value=points[i];
    int in = (compare_func==compare_y_points);
    unsigned int cifras = LL::to_string(max_value.point[in]).size();
    for(unsigned int i=0;i<cifras;i++)
    {
    	for(unsigned int j=0;j<index.size();++j)
    	{
    		int d = int(points[index[j]].point[in]/pow(10,i))%10;
    		queues[d].push(index[j]);
    	}
    	int p=0;
    	for(unsigned int j=0;j<queues.size();++j)
    	{
    		while(!queues[j].empty())
    		{
    			index[p]=queues[j].front();
    			queues[j].pop();
    			++p;
    		}
    	}
    }
    return index;
}

class Object
{
    private:
        unsigned int _V_id=0;
        float _V_pos_x=0;
        float _V_pos_y=0;
        LL_MathStructure::Polygon _V_polygon;
        LL_MathStructure::Point _V_min;
        LL_MathStructure::Point _V_max;
        LL_AL5::Text _V_text;
        LL_AL5::Rectangle _V_mbb;
    public:
        Object()
        {
            _V_text.set_flag(ALLEGRO_ALIGN_CENTER);
            _V_min.set_dimension(2);
            _V_max.set_dimension(2);
            _V_mbb.set_thickness(2);
        }
        void clear()
        {
            _V_pos_x=0;
            _V_pos_y=0;
            _V_polygon.clear();
            _V_min[0]=_V_min[1]=_V_max[0]=_V_max[1]=0;
        }
        unsigned int size()
        {
            return _V_polygon.size();
        }
        float get_pos_x()
        {
            return _V_pos_x;
        }
        float get_pos_y()
        {
            return _V_pos_y;
        }
        void set_pos(float pos_x,float pos_y)
        {
            _V_pos_x=pos_x;
            _V_pos_y=pos_y;
        }
        void set_text(std::string text)
        {
            _V_text=text;
        }
        LL_MathStructure::Point operator [](unsigned int i)
        {
            return _V_polygon[i];
        }
        void add_point(float x,float y)
        {
            LL_MathStructure::Point point;
            point.set_dimension(2);
            point[0]=x;
            point[1]=y;
            add_point(point);
        }
        void add_point(LL_MathStructure::Point point)
        {
            if(_V_polygon.add_point(point))
            {
                if(_V_polygon.size()>1)
                {
                    if(_V_min[0]>point[0])
                        _V_min[0]=point[0];
                    if(_V_max[0]<point[0])
                        _V_max[0]=point[0];
                    if(_V_min[1]>point[1])
                        _V_min[1]=point[1];
                    if(_V_max[1]<point[1])
                        _V_max[1]=point[1];
                }
                else
                {
                    _V_min=point;
                    _V_max=point;
                }
                _V_text.set_pos((_V_max[0]+_V_min[0])/2,(_V_max[1]+_V_min[1])/2);
                _V_mbb.set_pos(_V_min[0],_V_min[1]);
                _V_mbb.set_size_x(_V_max[0]-_V_min[0]);
                _V_mbb.set_size_y(_V_max[1]-_V_min[1]);
            }
        }
        MinMaxPoint get_point(MinMaxType type)
        {
            MinMaxPoint minmaxpoint;
            minmaxpoint.type=type;
            minmaxpoint.point=_V_min;
            if(type==MinMaxType::T_MAX)
                minmaxpoint.point=_V_max;
            minmaxpoint.point[0]+=_V_pos_x;
            minmaxpoint.point[1]+=_V_pos_y;
            return minmaxpoint;
        }
        LL_MathStructure::MBB get_mbb()
        {
            LL_MathStructure::MBB mbb(2);
            mbb.first_point  = _V_min;
            mbb.second_point = _V_max;
            return mbb;
        }
        void set_font(LL_AL5::Font* font)
        {
            _V_text.set_font(font);
        }
        void draw_object(LL_AL5::Display* display,LL_AL5::Color polygon_color,LL_AL5::Color mbb_color)
        {
            if(_V_polygon.size()<2)
                return;
            display->set_cam(-_V_pos_x,-_V_pos_y);
            _V_mbb.set_color(mbb_color);
            display->draw(&_V_mbb);
            if(draw_polygon)
            {
                LL_AL5::Line line;
                line.set_color(polygon_color);
                for(unsigned int i=0;i<_V_polygon.size();++i)
                {
                    unsigned int j=(i+1)%_V_polygon.size();
                    line.set_points(_V_polygon[i][0],_V_polygon[i][1],_V_polygon[j][0],_V_polygon[j][1]);
                    display->draw(&line);
                }
            }
            display->set_cam(0,0);
        }
        void draw_text(LL_AL5::Display* display,LL_AL5::Color text_color)
        {
            if(!_V_text.get_font())
                return;
            display->set_cam(-_V_pos_x,-_V_pos_y);
            _V_text.set_color(text_color);
            display->draw(&_V_text);
            display->set_cam(0,0);
        }
        void set_id(unsigned int id)
        {
            _V_id=id;
        }
        unsigned int get_id()
        {
            return _V_id;
        }
};

struct IndexMinMaxPoint
{
    MinMaxPoint point;
    unsigned int index;
    IndexMinMaxPoint()
    {
    }
    IndexMinMaxPoint(const MinMaxPoint& point,unsigned int index)
    {
        this->point=point;
        this->index=index;
    }
};

bool index_compare_x_points(const IndexMinMaxPoint& first, const IndexMinMaxPoint& second)
{
    return first.point.point[0]<second.point.point[0];
}

bool index_compare_y_points(const IndexMinMaxPoint& first, const IndexMinMaxPoint& second)
{
    return first.point.point[1]<second.point.point[1];
}

struct DataParallelCPU
{
    std::list<Object*>** objects;
    std::vector<int> total_collision;
    std::list<std::pair<int,int>> collision;
    int begin_index;
    int end_index;
    int max_x;
    int max_y;
    int thread_id;
};

#endif // INCLUDED_OBJECT_H
