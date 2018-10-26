#include <iostream>

#include <LexRisLogic/Convert.h>

#include "Scene.h"
#include "SAP_RTree1D.h"
#include "RTree2D.h"
#include "RStarTree2D.h"
#include "SAP_Unisize.h"
#include "SAP_IntervalTree.h"
#include "SAP_RStarTree1D.h"
#include "SAP_LL_IntervalTree.h"
#include "SAP.h"
#include "SAP_CPU_Parallel.h"
#include "SAP_LL_ITREE_Parallel.h"
#include "SAP_GPU_Parallel.h"
#include "SAP_GPU_LL_ITree_Parallel.h"

int main(int argc,char* argv[])
{
    bool print_collision=false;
    bool collision=false;
    bool render_frames=false;
    bool print_frames=true;
    LL::random_generate_new_seed();
    bool create_new_map=false;
    int mision=0;
    int max_test=-1;
    bool command=false;
    bool command_2=false;
    bool unisize=false;
    int total=-1;
    int threads=1;
    int parallel_x=1;
    int parallel_y=1;
    bool autosave=true;
    if(argc >= 6)
    {
        create_new_map=LL::to_int(argv[1]);
        mision=LL::to_int(argv[2]);
        max_test=LL::to_int(argv[3]);
        collision=LL::to_int(argv[4]);
        autosave=LL::to_int(argv[5]);
        int counter=6;
        if(create_new_map)
        {
            if(argc>counter)
            {
                unisize=LL::to_int(argv[counter++]);
                total=LL::to_int(argv[counter++]);
                command=true;
            }
        }
        if(argc>counter)
        {
            parallel_x=LL::to_int(argv[counter++]);
            parallel_y=LL::to_int(argv[counter++]);
            threads=LL::to_int(argv[counter++]);
            command_2=true;
        }
    }
    else if(argc==1)
    {
        std::cout<<"Nuevo Mapa (1/0): ";
        std::cin>>create_new_map;
        std::cout<<"AutoSave (1/0): ";
        std::cin>>autosave;
    }
    else
    {
        std::cout<<"SaP <CREATE_NEW_MAP> <ALGORITHM> <TEST> <START> <AUTOSAVE> [<UNISIZE> <TOTAL>] ";
        std::cout<<"[<PARALLEL_X> <PARALLEL_Y> <THREADS>]"<<std::endl;
        return -1;
    }
    Scene scene("scene.txt");
    scene.load();
    if(create_new_map)
    {
        if(!command)
        {
            std::cout<<"Unisize (1/0): ";
            std::cin>>unisize;
        }
        while(total <= 0)
        {
            std::cout<<"Total de Elementos: ";
            std::cin>>total;
        }
        scene.create(unisize,total);
        scene.save();
    }
    if(max_test == 0)
        return 0;
    std::string name_function;
    void (*collision_function)(
                               std::vector<Object>&,
                               std::vector<int>&,
                               std::list<std::pair<int,int>>&,
                               float*,
                               float*
                               )=nullptr;
    void (*collision_function_2)(
                               std::vector<Object*>**,
                               std::vector<int>&,
                               std::list<std::pair<int,int>>&,
                               float*,
                               float*,
                               int,
                               float,
                               float
                               )=nullptr;
    void (*collision_function_3)(
                               float*,
                               int*,
                               unsigned int,
                               std::vector<int>&,
                               std::list<std::pair<int,int>>&,
                               float*,
                               float*,
                               int,
                               unsigned int,
                               unsigned int,
                               unsigned int,
                               unsigned int
                               )=nullptr;
    while(1)
    {
        if(mision==1)
        {
            name_function="SAP RTree 1D";
            collision_function=SAP_RTree1D;
            break;
        }
        else if(mision==2)
        {
            name_function="RTree 2D";
            collision_function=RTree2D;
            break;
        }
        else if(mision==3)
        {
            name_function="SAP Unisize BOX";
            collision_function=SAP_Unisize_Box;
            break;
        }
        else if(mision==4)
        {
            name_function="SAP Interval Tree";
            collision_function=SAP_IntervalTree;
            break;
        }
        else if(mision==5)
        {
            name_function="RStarTree 2D";
            collision_function=RStarTree2D;
            break;
        }
        else if(mision==6)
        {
            name_function="SAP RStarTree 1D";
            collision_function=SAP_RStarTree1D;
            break;
        }
        else if(mision==7)
        {
            name_function="SAP LL Interval Tree";
            collision_function=SAP_LL_IntervalTree;
            break;
        }
        else if(mision==8)
        {
            name_function="SAP Original";
            collision_function=SAP;
            break;
        }
        else if(mision==9)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP Parallel";
            collision_function_2=SAP_CPU_Parallel;
            scene.build(parallel_x,parallel_y);
            break;
        }
        else if(mision==10)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP LL_INtervalTree Parallel";
            collision_function_2=SAP_LL_ITREE_CPU_Parallel;
            scene.build(parallel_x,parallel_y);
            break;
        }
        else if(mision==11)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP GPU Parallel";
            collision_function_3=SAP_GPU_Parallel;
            scene.build_gpu(parallel_x,parallel_y);
            build_sap_gpu_parallel(scene.get_local_size(),scene.size());
            break;
        }
        else if(mision==12)
        {
            if(!command_2)
            {
                std::cout<<"Parallel X: ";
                std::cin>>parallel_x;
                std::cout<<"Parallel Y: ";
                std::cin>>parallel_y;
                std::cout<<"Threads: ";
                std::cin>>threads;
            }
            name_function="SAP GPU LL ITREE Parallel";
            collision_function_3=SAP_GPU_LL_ITree_Parallel;
            scene.build_gpu(parallel_x,parallel_y);
            build_sap_gpu_ll_parallel(scene.get_local_size(),scene.size());
            break;
        }
        std::cout<<"Ingresar Funcion:\n1: SAP_RTree1D\n2: RTree2D\n3: SAP_Unisize_Box\n";
        std::cout<<"4: SAP_IntervalTree\n5: R*Tree2D\n6: SAP_R*Tree1D\n7: SAP_LL_IntervalTree\n";
        std::cout<<"8: SAP\n9: SAP CPU Parallel\n10: SAP LL_IntervalTree CPU Parallel\n";
        std::cout<<"11: SAP GPU Parallel\n12: SAP LL_IntervalTree GPU Parallel\n";
        std::cout<<"Opcion:";
        std::cin>>mision;
    }
    std::vector<int> on_collision(scene.size(),false);
    LL_AL5::init_allegro();
    LL_AL5::primitives_addon();
    LL_AL5::text_addon();
    LL_AL5::Display display(1366,768,SCENE_SIZE_X,SCENE_SIZE_Y);
    display.set_display_mode(ALLEGRO_FULLSCREEN_WINDOW);
    display.create();
    LL_AL5::Font font;
    font.set_path("comic.ttf");
    font.set_size(SCENE_SIZE_X/100.0);
    font.load_ttf_font();
    LL_AL5::Font another_font;
    another_font.set_path("comic.ttf");
    another_font.set_size(12.0);
    another_font.load_ttf_font();
    scene.set_font(&font);
    LL_AL5::KeyControl key_control;
    key_control.add_key("Polygon",ALLEGRO_KEY_P);
    key_control.add_key("Frames",ALLEGRO_KEY_F);
    key_control.add_key("Render",ALLEGRO_KEY_S);
    key_control.add_key("Collision",ALLEGRO_KEY_C);
    key_control.add_key("Print Collision",ALLEGRO_KEY_SPACE);
    key_control.add_key("Controls",ALLEGRO_KEY_L);
//    std::cout<<"Controls:"<<std::endl;
//    std::cout<<"Show/Hide Polygon: P"<<std::endl;
//    std::cout<<"Show/Hide Text: F"<<std::endl;
//    std::cout<<"Render Object: S"<<std::endl;
//    std::cout<<"Show Controls: L"<<std::endl;
//    std::cout<<"Make Collision: C"<<std::endl;
//    std::cout<<"Print Collision Info: SPACE"<<std::endl;
//    std::cout<<"--------------------"<<std::endl;
    int total_frames=0;
    LL::Chronometer time;
    std::list<std::pair<int,int>> collision_list;
    time.play();
    LL_AL5::Input input;
    input.register_display(display);
    input.keyboard_on();
    input.set_key_control(&key_control);
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
    total_text=LL::to_string(scene.size());
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
    std::cout<<"Ready"<<std::endl;
    std::list<std::pair<float,float>> tiempos;
    while(!input.get_display_status() && test!=max_test)
    {
        ++total_frames;
        display.clear();
        scene.draw_scene(&display);
        if(render_frames)
        {
            for(unsigned int i=0;i<scene.size();++i)
            {
                switch(on_collision[i])
                {
                    case 1:
                    {
                        color.red   = 0;
                        color.green = 0;
                        color.blue  = 255;
                        break;
                    }
                    case 0:
                    {
                        color.red   = 0;
                        color.green = 0;
                        color.blue  = 0;
                        break;
                    }
                    default:
                    {
                        color.red   =255;
                        color.green =0;
                        color.blue  =0;
                        break;
                    }
                }
                scene[i].draw_object(&display,green,color);
            }
            for(unsigned int i=0;i<scene.size();++i)
                scene[i].draw_text(&display,black);
        }
        if(print_frames)
        {
            display.draw(&fps_text,false);
            display.draw(&total_text,false);
            display.draw(&collision_text,false);
            display.draw(&min_time_text,false);
            display.draw(&max_time_text,false);
            display.draw(&prom_time_text,false);
            display.draw(&total_test_text,false);
        }
        display.refresh();
        if(input.get_event())
        {
            if(input["Collision"])
            {
                collision=!collision;
                input["Collision"]=false;
            }
            if(input["Print Collision"])
            {
                print_collision=!print_collision;
                input["Print Collision"]=false;
            }
            if(input["Polygon"])
            {
                draw_polygon=!draw_polygon;
                input["Polygon"]=false;
            }
            if(input["Controls"])
            {
                std::cout<<"Controls:"<<std::endl;
                std::cout<<"Show/Hide Polygon: P"<<std::endl;
                std::cout<<"Show/Hide Text: F"<<std::endl;
                std::cout<<"Render Object: S"<<std::endl;
                std::cout<<"Show Controls: L"<<std::endl;
                std::cout<<"Make Collision: C"<<std::endl;
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
        }
        if(collision)
        {
            collision_list.clear();
            for(unsigned int i=0;i<scene.size();++i)
                on_collision[i]=0;
            float time_construction;
            float time_collision;
            if(collision_function)
            {
                collision_function(scene.get_objects(),
                                   on_collision,
                                   collision_list,
                                   &time_construction,
                                   &time_collision);
            }
            else if(collision_function_2)
            {
                collision_function_2(scene.get_objects_grid(),
                                     on_collision,
                                     collision_list,
                                     &time_construction,
                                     &time_collision,
                                     threads,
                                     scene.size_x(),
                                     scene.size_y());
            }
            else if(collision_function_3)
            {
                collision_function_3(scene.get_objects_gpu_grid(),
                                     scene.get_sizes_gpu_grid(),
                                     scene.size(),
                                     on_collision,
                                     collision_list,
                                     &time_construction,
                                     &time_collision,
                                     threads,
                                     scene.size_x(),
                                     scene.size_y(),
                                     scene.get_size_gpu_grid(),
                                     scene.get_local_size());
            }
            float total_time=time_construction+time_collision;
            tiempos.push_back(std::pair<float,float>(time_construction,time_collision));
            total_test_text=LL::to_string(++test);
            collision_text=LL::to_string(total_time)+" s";
            acum+=total_time;
            if(min_time>total_time)
            {
                min_time=total_time;
                min_time_text=LL::to_string(min_time)+" s";
            }
            if(max_time<total_time)
            {
                max_time=total_time;
                max_time_text=LL::to_string(max_time)+" s";
            }
            prom_time_text=LL::to_string(acum/test)+" s";
        }
        if(print_collision)
        {
            system("cls");
            std::cout<<"--------------------"<<std::endl;
            collision_list.sort();
            for(auto i=collision_list.begin();i!=collision_list.end();++i)
                std::cout<<"("<<(*i).first<<","<<(*i).second<<")"<<std::endl;
            std::cout<<"--------------------"<<std::endl;
            print_collision=false;
        }
        if(time.get_time()>1)
        {
            fps_text=LL::to_string(total_frames/time.get_time())+" fps";
            time.clear();
            total_frames=0;
        }
    }
    input.unregister_display();
    input.unregister_timer();
    if(test)
    {
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<"Total:     "<<scene.size()<<std::endl;
        std::cout<<"Test:      "<<test<<std::endl;
        std::cout<<"Algorithm: "<<name_function<<std::endl;
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<"Min:       "<<min_time<<std::endl;
        std::cout<<"Max:       "<<max_time<<std::endl;
        std::cout<<"Prom:      "<<acum/test<<std::endl;
        std::cout<<"_________________________________________________"<<std::endl;
        std::cout<<std::endl;
    }
    if(autosave)
    {
        LL::FileStream txt_times;
        std::string path_name=name_function+" (S="+LL::to_string(scene.size())+").csv";
        txt_times.set_path(path_name);
        txt_times.load();
        txt_times.clear_file();
        txt_times.insert_line(0,tiempos.size());
        unsigned int index=0;
        for(auto tiempo:tiempos)
        {
            txt_times[index]=LL::to_string(tiempo.first)+";"+LL::to_string(tiempo.second)+";";
            std::replace(txt_times[index].begin(),txt_times[index].end(),'.',',');
            ++index;
        }
        std::cout<<"Saving: "<<path_name<<std::endl;
        txt_times.save();
    }
    delete_sap_gpu_parallel();
    delete_sap_gpu_ll_parallel();
    return 0;
}
