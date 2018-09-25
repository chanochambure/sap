#ifndef SAP_CPU_PARALLEL_H_INCLUDED
#define SAP_CPU_PARALLEL_H_INCLUDED

#include "Object.h"

void SAP_CPU_Parallel(std::list<Object*>** objects,
                      std::vector<int>& total_collision,
                      std::list<std::pair<int,int>>& collision,
                      float* time_construction,float* time_collision)
{
    LL::Chronometer chronometer;
    chronometer.play();
    //Construction BEGIN
    //Construction END
    chronometer.stop();
    if(time_construction)
        *time_construction=chronometer.get_time();
    chronometer.play();
    //Collision BEGIN
    //Collision END
    chronometer.stop();
    if(time_collision)
        *time_collision=chronometer.get_time();
}

#endif // SAP_CPU_PARALLEL_H_INCLUDED
