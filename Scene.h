#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED

#include <LexRisLogic/MathStructures/Point.h>
#include <LexRisLogic/FileStream.h>
#include <LexRisLogic/StringSplitter.h>
#include <LexRisLogic/Math.h>

#include <vector>

#include "Object.h"

const unsigned int SCENE_SIZE_X=750;
const unsigned int SCENE_SIZE_Y=450;

class Scene
{
    private:
        LL::FileStream _V_file;
        std::vector<Object> _V_objects;
        std::list<Object*>** _V_grid=nullptr;
        unsigned int _V_size_x=0;
        unsigned int _V_size_y=0;
        void _F_delete_grid()
        {
            if(_V_grid)
            {
                for(unsigned int i=0;i<_V_size_x;++i)
                {
                    delete[](_V_grid[i]);
                    _V_grid[i]=nullptr;
                }
                delete[](_V_grid);
            }
            _V_grid=nullptr;
        }
    public:
        Scene(std::string file_path)
        {
            _V_file.set_path(file_path);
        }
        bool build(int division_x,int division_y)
        {
            _F_delete_grid();
            if(_V_objects.size())
            {
                _V_size_x=division_x;
                _V_size_y=division_y;
                _V_grid=new std::list<Object*>*[_V_size_x];
                for(unsigned int i=0;i<_V_size_x;++i)
                    _V_grid[i]=new std::list<Object*>[_V_size_y];
                for(auto i=_V_objects.begin();i!=_V_objects.end();++i)
                {
                    Object& obj=(*i);
                    LL_MathStructure::MBB mbb=obj.get_mbb();
                    int min_px=LL::max_integer(mbb.first_point[0]/SCENE_SIZE_X);
                    int min_py=LL::max_integer(mbb.first_point[1]/SCENE_SIZE_Y);
                    int max_px=LL::max_integer(mbb.second_point[0]/SCENE_SIZE_X);
                    int max_py=LL::max_integer(mbb.second_point[1]/SCENE_SIZE_Y);
                    for(int x=min_px;x<=max_px;++x)
                    {
                        for(int y=min_py;y<=max_py;++y)
                            _V_grid[x][y].push_back(&obj);
                    }
                }
                return true;
            }
            return false;
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
                object.set_pos(LL::random(0,SCENE_SIZE_X-40),LL::random(0,SCENE_SIZE_Y-40));
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
        std::list<Object*>** get_objects_grid()
        {
            return _V_grid;
        }
        unsigned int size_x()
        {
            return _V_size_x;
        }
        unsigned int size_y()
        {
            return _V_size_y;
        }
        ~Scene()
        {
            _F_delete_grid();
            clear();
        }
};

#endif // SCENE_H_INCLUDED
