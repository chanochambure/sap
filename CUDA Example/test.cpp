#include "stdio.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <math.h>
#define __device__
// #include <LexRisLogic/Convert.h>

using namespace std;

//#include "GPURAM.h"

#include "GPU/RadixSort.h"
//#include "Scene.h"
#include "GPU/Interval.h"
#include "GPU/IntervalTree.h"


int main()
{
	IntervalTree S=IntervalTree_create();
	for(unsigned int i=0;i<100;++i)
        std::cout<<IntervalTree_insert(&S,Interval_create(5,i))<<std::endl;
    List result=IntervalTree_range_query(&S,Interval_create(-1,6));
    for(struct ListNode* d=result.root;d;d=d->next)
    {
        cout<<d->interval.data[0]<<" "<<d->interval.data[1]<<endl;
    }
    List_clear(&result);
	IntervalTree_clear(&S);
	return 0;
}
