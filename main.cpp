#include "Object.h"

#include "DataStructure/IntervalTree.h"
#include <LexRisLogic/DataStructures/RTree.h>

using namespace std;

LL_MathStructure::MBB pair_to_mbb(Interval interval)
{
    LL_MathStructure::MBB mbb;
    mbb.set_dimension(1);
    mbb.first_point[0]=interval.first;
    mbb.second_point[0]=interval.second;
    return mbb;
}

bool compare_x_points(const MinMaxPoint& first, const MinMaxPoint& second)
{
    return first.point[0]<second.point[0];
}

bool compare_y_points(const MinMaxPoint& first, const MinMaxPoint& second)
{
    return first.point[1]<second.point[1];
}

vector<int> LSDRS(vector<MinMaxPoint>& points,bool(*compare_func)(const MinMaxPoint&,const MinMaxPoint&))
{
    vector<int> index(points.size());
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

void fast_collision_SaP(vector<Object>& objects,
                        vector<int>& total_collision,list<pair<int,int>>& collision)
{
    int n=objects.size();
    int n_2=objects.size()*2;
    collision.clear();
    for(int i=0;i<n;++i)
        total_collision[i]=0;
    vector<MinMaxPoint> points(n_2);
    for(int i=0;i<n;++i)
    {
        points[i]=objects[i].get_point(MinMaxType::T_MIN);
        points[i+n]=objects[i].get_point(MinMaxType::T_MAX);
    }
    vector<int> R(n_2);
    vector<int> iX=LSDRS(points,compare_x_points);
    for(int i=0;i<n_2;++i)
        R[iX[i]]=i;
    vector<int> iY=LSDRS(points,compare_y_points);
//    std::map<int,int> S;
    LL_DataStructure::RTree<Interval,1,3> S(pair_to_mbb);
    for(int i=0;i<n_2;++i)
    {
        int p=iY[i];
        if(p<n)
        {
//            S[R[p]]=1;
//            S[R[p+n]]=1;
            int mini=R[p];
            int maxi=R[p+n];
            if(mini>maxi)
                swap(mini,maxi);
            Interval intervalo(mini,maxi);
            S.insert(intervalo);
//            std::cout<<"ADD- "<<p;
//            std::cout<<std::endl;
            std::list<Interval> result=S.range_query(pair_to_mbb(intervalo));
            for(auto d=result.begin();d!=result.end();++d)
            {
                if(*d==intervalo)
                    continue;
                int index_i=p;
                int index_j=iX[(d->first)]%n;
                ++total_collision[index_i];
                ++total_collision[index_j];
                collision.push_back(pair<int,int>(index_i,index_j));
//                std::cout<<"\t- "<<index_j;
//                std::cout<<"\n";
            }
//            auto result_f=S.upper_bound(mini);
//            for(auto i=result_f;i!=S.end() && i->first<maxi;++i)
//            {
//                int index_i=p%n;
//                int index_j=iX[(i->first)]%n;
//                ++total_collision[index_i];
//                ++total_collision[index_j];
//                collision.push_back(pair<int,int>(index_i,index_j));
//            }
        }
        else
        {
            int mini=R[p-n];
            int maxi=R[p];
            if(mini>maxi)
                swap(mini,maxi);
            Interval intervalo(mini,maxi);
            S.remove(intervalo);
//            std::cout<<"REM- "<<p-n;
//            std::cout<<std::endl;
//            auto rpn=S.find(R[p-n]);
//            auto rp=S.find(R[p]);
//            if(rpn!=S.end())
//                S.erase(rpn);
//            if(rp!=S.end())
//                S.erase(rp);
        }
    }
//    std::cout<<std::endl;
//    std::cout<<std::endl;
}

int main(int argc,char* argv[])
{/*
    IntervalTree tree;
    tree.insert(Interval(2,1));
    tree.insert(Interval(1,1));
    tree.insert(Interval(12,1));
    tree.insert(Interval(1,-1));
    tree.insert(Interval(1,231));
    print(tree.get_intervals());*/
    float S_X=500;
    float S_Y=300;
    float space_of_object=0.2;
    unsigned int total=1;
    int total_test=100000;
    if(argc==6)
    {
        S_X=LL::to_float(argv[1]);
        S_Y=LL::to_float(argv[2]);
        space_of_object=LL::to_float(argv[3]);
        total=LL::to_int(argv[4]);
        total_test=LL::to_int(argv[5]);
    }
    else
        std::cout<<"SaP <Size X> <Size Y> <Object Space> <elements> <tests>"<<std::endl;
    list<pair<int,int>> collision_list;
    draw_polygon=false;
    LL_MathStructure::Point point(2);
//    Object object;
//    point[0]=0;point[1]=0;
//    object.add_point(point);
//    point[0]=S_X*space_of_object;point[1]=S_Y*space_of_object;
//    object.add_point(point);
//    point[0]=S_X*space_of_object;point[1]=0;
//    object.add_point(point);
    LL::random_generate_new_seed();
    LL_AL5::init_allegro();
    LL_AL5::primitives_addon();
    LL_AL5::text_addon();
    LL_AL5::Display display(1366,768,S_X,S_Y);
    display.set_display_mode(ALLEGRO_FULLSCREEN_WINDOW);
    display.create();
    LL_AL5::Font font;
    font.set_path("comic.ttf");
    font.set_size(S_X/100.0);
    font.load_ttf_font();
    LL_AL5::Font another_font;
    another_font.set_path("comic.ttf");
    another_font.set_size(S_X/50.0);
    another_font.load_ttf_font();
//    object.set_font(&font);
    vector<Object> objects(total);
    vector<int> on_collision(total,false);
    for(unsigned int i=0;i<total;++i)
    {
        objects[i].set_pos(LL::random(40,S_X*(1.0-space_of_object)),
                           LL::random(40,S_Y*(1.0-space_of_object)));
        objects[i].set_font(&font);
        objects[i].set_text(LL::to_string(i));
        unsigned int sides = LL::random(3,6);
        for(unsigned int side=0;side<sides;++side)
        {
            point[0]=LL::random(-40,40);
            point[1]=LL::random(-40,40);
            objects[i].add_point(point);
        }
    }
    LL_AL5::KeyControl key_control;
    key_control.add_key("Collision",ALLEGRO_KEY_C);
    key_control.add_key("Polygon",ALLEGRO_KEY_P);
    key_control.add_key("Frames",ALLEGRO_KEY_F);
    key_control.add_key("Render",ALLEGRO_KEY_S);
    key_control.add_key("Add",ALLEGRO_KEY_A);
    key_control.add_key("Position",ALLEGRO_KEY_O);
    key_control.add_key("Print Collision",ALLEGRO_KEY_SPACE);
    key_control.add_key("Controls",ALLEGRO_KEY_L);
    std::cout<<"Controles:"<<std::endl;
    std::cout<<"Make Collision: C"<<std::endl;
    std::cout<<"Show Polygon: P"<<std::endl;
    std::cout<<"Show Frames: F"<<std::endl;
    std::cout<<"Render Object: S"<<std::endl;
    std::cout<<"Add Random Object: A"<<std::endl;
    std::cout<<"Change Position: O"<<std::endl;
    std::cout<<"Print Collision Info: SPACE"<<std::endl;
    std::cout<<"--------------------"<<std::endl;
    int total_frames=0;
    LL::Chronometer time;
    time.play();
    LL_AL5::Input input;
    input.register_display(display);
    input.keyboard_on();
    input.set_key_control(&key_control);
    bool collision=true;
    bool print_collision=false;
    bool render_frames=false;
    bool print_frames=true;
    LL_AL5::Color black;
    LL_AL5::Color green(0,255);
    LL_AL5::Color color;
    LL_AL5::Text fps_text;
    fps_text.set_font(&another_font);
    fps_text.set_color(black);
    fps_text.set_pos(10,10);
    fps_text="0 fps";
    LL_AL5::Text collision_text;
    collision_text.set_font(&another_font);
    collision_text.set_color(black);
    collision_text.set_pos(10,10+2*another_font.get_size());
    collision_text="0 s";
    LL_AL5::Text total_text;
    total_text.set_font(&another_font);
    total_text.set_color(black);
    total_text.set_pos(10,10+4*another_font.get_size());
    total_text=LL::to_string(total);
    LL_AL5::Text max_time_text;
    max_time_text.set_font(&another_font);
    max_time_text.set_color(black);
    max_time_text.set_pos(10,10+9*another_font.get_size());
    max_time_text="0 s";
    LL_AL5::Text min_time_text;
    min_time_text.set_font(&another_font);
    min_time_text.set_color(black);
    min_time_text.set_pos(10,10+7*another_font.get_size());
    min_time_text="0 s";
    LL_AL5::Text prom_time_text;
    prom_time_text.set_font(&another_font);
    prom_time_text.set_color(black);
    prom_time_text.set_pos(10,10+11*another_font.get_size());
    prom_time_text="0 s";
    LL_AL5::Text total_test_text;
    total_test_text.set_font(&another_font);
    total_test_text.set_color(black);
    total_test_text.set_pos(10,10+14*another_font.get_size());
    total_test_text="0";
    double min_time=1000;
    double max_time=0;
    double acum=0;
    int test=0;
    LL::Chronometer time_collision;
	std::vector<float> tiempos(total_test,10000);
    while(!input.get_display_status())
    {
        ++total_frames;
        display.clear();
        if(render_frames)
        {
            for(unsigned int i=0;i<total;++i)
            {
                color.red   =(on_collision[i]*11)%255;
                color.green =(on_collision[i]*41)%255;
                color.blue  =(on_collision[i]*73)%255;
                objects[i].draw_object(&display,green,color);
            }
            for(unsigned int i=0;i<total;++i)
                objects[i].draw_text(&display,black);
        }
        if(print_frames)
            display.draw(&fps_text,false);
        display.draw(&total_text,false);
        display.draw(&collision_text,false);
        display.draw(&min_time_text,false);
        display.draw(&max_time_text,false);
        display.draw(&prom_time_text,false);
        display.draw(&total_test_text,false);
        display.refresh();
        if(input.get_event())
        {
            if(input["Collision"])
            {
                collision=!collision;
                if(!collision)
                {
                    collision_list.clear();
                    for(unsigned int i=0;i<on_collision.size();++i)
                        on_collision[i]=0;
                }
                input["Collision"]=false;
            }
            if(input["Print Collision"])
            {
                print_collision=!print_collision;
                input["Print Collision"]=false;
            }
            if(input["Add"])
            {
                for(unsigned int i=0;i<1;++i)
                {
                    ++total;
                    on_collision.push_back(0);
                    Object object;
                    object.set_pos(LL::random(40,S_X*(1.0-space_of_object)),
                                   LL::random(40,S_Y*(1.0-space_of_object)));
                    object.set_font(&font);
                    object.set_text(LL::to_string(objects.size()));
                    unsigned int sides = LL::random(3,6);
                    for(unsigned int side=0;side<sides;++side)
                    {
                        point[0]=LL::random(-40,40);
                        point[1]=LL::random(-40,40);
                        object.add_point(point);
                    }
                    objects.push_back(object);
                }
                total_text=LL::to_string(total);
                //input["Add"]=false;
            }
            if(input["Polygon"])
            {
                draw_polygon=!draw_polygon;
                input["Polygon"]=false;
            }
            if(input["Controls"])
            {
                std::cout<<"Controles:"<<std::endl;
                std::cout<<"Make Collision: C"<<std::endl;
                std::cout<<"Show Polygon: P"<<std::endl;
                std::cout<<"Show Frames: F"<<std::endl;
                std::cout<<"Render Object: S"<<std::endl;
                std::cout<<"Add Random Object: A"<<std::endl;
                std::cout<<"Change Position: O"<<std::endl;
                std::cout<<"Print Collision Info: SPACE"<<std::endl;
                std::cout<<"--------------------"<<std::endl;
                input["Controls"]=false;
            }
            if(input["Frames"])
            {
                print_frames=!print_frames;
                input["Frames"]=false;
            }
            if(input["Render"])
            {
                render_frames=!render_frames;
                input["Render"]=false;
            }
            if(input["Position"])
            {
                for(unsigned int i=0;i<on_collision.size();++i)
                    on_collision[i]=0;
                for(unsigned int i=0;i<total;++i)
                {
                    objects[i].set_pos(LL::random(40,S_X*(1.0-space_of_object)),
                                       LL::random(40,S_Y*(1.0-space_of_object)));
                }
                input["Position"]=false;
            }
        }
        if(collision)
        {
            time_collision.play();
            fast_collision_SaP(objects,on_collision,collision_list);
            time_collision.stop();
            tiempos[test]=time_collision.get_time();
            total_test_text=LL::to_string(++test);
            collision_text=LL::to_string(time_collision.get_time())+" s";
            acum+=time_collision.get_time();
            if(min_time>time_collision.get_time())
            {
                min_time=time_collision.get_time();
                min_time_text=LL::to_string(min_time)+" s";
            }
            if(max_time<time_collision.get_time())
            {
                max_time=time_collision.get_time();
                max_time_text=LL::to_string(max_time)+" s";
            }
            prom_time_text=LL::to_string(acum/test)+" s";
            if(test>=total_test)
                break;
        }
        if(print_collision)
        {
            system("cls");
            std::cout<<"--------------------"<<std::endl;
            for(auto i=collision_list.begin();i!=collision_list.end();++i)
                std::cout<<"("<<(*i).first<<","<<(*i).second<<")"<<std::endl;
            std::cout<<"--------------------"<<std::endl;
            print_collision=false;
        }
        if(time.get_time()>1)
        {
            time.clear();
            fps_text=LL::to_string(total_frames)+" fps";
            total_frames=0;
        }
    }
    input.unregister_display();
    input.unregister_timer();
    std::cout<<"n: "<<total<<std::endl;
    std::cout<<"d: "<<space_of_object<<std::endl;
    std::cout<<"min: "<<min_time<<std::endl;
    std::cout<<"max: "<<max_time<<std::endl;
    std::cout<<"prom: "<<acum/test<<std::endl;
    sort(tiempos.begin(), tiempos.end());
    std::cout<<"med: "<<tiempos[test/2]<<std::endl;
    std::cout<<"test: "<<test<<std::endl;
    std::cout<<std::endl;
    return 0;
}
