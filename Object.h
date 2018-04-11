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

class Object
{
    private:
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
        void set_pos(float pos_x,float pos_y)
        {
            _V_pos_x=pos_x;
            _V_pos_y=pos_y;
        }
        void set_text(std::string text)
        {
            _V_text=text;
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
};

#endif // INCLUDED_OBJECT_H
