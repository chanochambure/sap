#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include <LexRisLogic/MathStructures/Point.h>
#include <LexRisLogic/FileStream.h>
#include <LexRisLogic/StringSplitter.h>
#include <LexRisLogic/Math.h>

#include <vector>

#include "Object.h"

const unsigned int SCENE_SIZE_X=800;
const unsigned int SCENE_SIZE_Y=450;

class Scene
{
    private:
        LL::FileStream _V_file;
        std::vector<Object> _V_objects;
    public:
        Scene(std::string file_path)
        {
            _V_file.set_path(file_path);
        }
        void load()
        {
            _V_file.load();
            _V_objects=std::vector<Object>(_V_file.size(),Object());
            LL::StringSplitter splitter;
            for(unsigned int i=0;i<_V_file.size();++i)
            {
                splitter.set_string(_V_file[i]);
                splitter.split(' ');
                _V_objects[i].set_pos(LL::to_float(splitter[0]),LL::to_float(splitter[1]));
                for(unsigned int j=2;j<splitter.size();j+=2)
                    _V_objects[i].add_point(LL::to_float(splitter[j]),LL::to_float(splitter[j+1]));
                _V_objects[i].set_text(LL::to_string(i));
                _V_objects[i].set_id(i);
            }
        }
        void set_font(LL_AL5::Font* font)
        {
            for(auto i=_V_objects.begin();i!=_V_objects.end();++i)
                i->set_font(font);
        }
        void clear()
        {
            _V_file.clear_file();
            _V_objects.clear();
        }
        void create(bool unisize,unsigned int total)
        {
            clear();
            _V_objects=std::vector<Object>(total,Object());
            Object object;
            object.add_point(0,15);
            object.add_point(15,0);
            object.add_point(30,15);
            object.add_point(15,30);
            for(unsigned int i=0;i<total;++i)
            {
                if(!unisize)
                {
                    object.clear();
                    unsigned int sides = LL::random(3,6);
                    for(unsigned int side=0;side<sides;++side)
                        object.add_point(LL::random(0,30),LL::random(0,30));
                }
                object.set_id(i);
                object.set_pos(LL::random(0,SCENE_SIZE_X-30),LL::random(0,SCENE_SIZE_Y-30));
                object.set_text(LL::to_string(i));
                _V_objects[i]=object;
            }
        }
        void save()
        {
            _V_file.clear_file();
            _V_file.insert_line(0,_V_objects.size());
            for(unsigned int i=0;i<_V_objects.size();++i)
            {
                std::string line;
                line+=LL::to_string(_V_objects[i].get_pos_x());
                line+=" ";
                line+=LL::to_string(_V_objects[i].get_pos_y());
                for(unsigned int j=0;j<_V_objects[i].size();++j)
                {
                    line+=" ";
                    line+=LL::to_string(_V_objects[i][j][0]);
                    line+=" ";
                    line+=LL::to_string(_V_objects[i][j][1]);
                }
                _V_file[i]=line;
            }
            _V_file.save();
        }
        Object& operator [] (unsigned int i)
        {
            return _V_objects[i];
        }
        unsigned int size()
        {
            return _V_objects.size();
        }
        std::vector<Object>& get_objects()
        {
            return _V_objects;
        }
};

#endif // SCENE_H_INCLUDED
