/* MBB.cpp -- Minimal Bounding Box Math Structure Source - LexRis Logic Headers

    Copyright (c) 2017-2018 LexRisLogic

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software && associated
    documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, && to
    permit persons to whom the Software is furnished to do so.

    The above copyright notice && this permission notice shall be included in all copies || substantial portions of
    the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS || IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE && NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS || COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES || OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT || OTHERWISE, ARISING FROM, OUT OF || IN CONNECTION WITH THE SOFTWARE || THE USE || OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "../../include/LexRisLogic/MathStructures/MBB.h"

namespace LL_MathStructure
{
    MBB::MBB()
    {
    }
    MBB::MBB(unsigned int new_dimension)
    {
        set_dimension(new_dimension);
    }
    bool MBB::set_dimension(unsigned int new_dimension)
    {
        if(new_dimension)
        {
            dimension=new_dimension;
            first_point.set_dimension(dimension);
            second_point.set_dimension(dimension);
            return true;
        }
        return false;
    }
    bool MBB::operator == (MBB another_mbb)
    {
        return ((first_point==another_mbb.first_point) && (second_point==another_mbb.second_point));
    }
    bool MBB::operator != (MBB another_mbb)
    {
        return ((first_point!=another_mbb.first_point) || (second_point!=another_mbb.second_point));
    }

    double LL_SHARED mbb_distance(MBB first_mbb,MBB second_mbb)
    {
        if(first_mbb.dimension==second_mbb.dimension)
        {
            double acumulator=0.0;
            for(unsigned int i=0;i<first_mbb.dimension;++i)
            {
                double min_distance=std::max(first_mbb.first_point[i],second_mbb.first_point[i])-
                                    std::min(first_mbb.second_point[i],second_mbb.second_point[i]);
                if(min_distance>0)
                    acumulator+=pow(min_distance,2);
            }
            return sqrt(acumulator);
        }
        return 0.0;
    }

    bool LL_SHARED mbb_collision(MBB first_mbb,MBB second_mbb)
    {
        if(first_mbb.dimension!=second_mbb.dimension)
            return false;
        for(unsigned int i=0;i<first_mbb.dimension;++i)
        {
            float ini_segment=std::max(first_mbb.first_point[i],second_mbb.first_point[i]);
            float fin_segment=std::min(first_mbb.second_point[i],second_mbb.second_point[i]);
            if(ini_segment>fin_segment)
                return false;
        }
        return true;
    }
}
