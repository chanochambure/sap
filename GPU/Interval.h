#ifndef INTERVAL_H_INCLUDED
#define INTERVAL_H_INCLUDED

struct Interval
{
    float data[2];
};

Interval Interval_create(float left,float right)
{
    Interval newInterval;
    newInterval.data[left>right]=left;
    newInterval.data[left<=right]=right;
    return newInterval;
}
float Interval_get_mid_point(Interval interval)
{
    return (interval.data[0]+interval.data[1])/2.0;
}
bool Interval_contains(Interval interval,float point)
{
    return interval.data[0]<=point && point<=interval.data[1];
}
bool Interval_contains(Interval interval,Interval another_interval)
{
    return interval.data[0]<=another_interval.data[0] && another_interval.data[1]<=interval.data[1];
}
bool Interval_intersects(Interval interval,Interval another_interval)
{
    float ini_point=max(interval.data[0],another_interval.data[0]);
    float end_point=min(interval.data[1],another_interval.data[1]);
    return ini_point<=end_point;
}
bool Interval_is_left_of(Interval interval,float point)
{
    return interval.data[1]<point;
}
bool Interval_is_right_of(Interval interval,float point)
{
    return point<interval.data[0];
}
bool Interval_is_left_of(Interval interval,Interval another_interval)
{
    return Interval_is_left_of(interval,another_interval.data[0]);
}
bool Interval_is_right_of(Interval interval,Interval another_interval)
{
    return Interval_is_right_of(interval,another_interval.data[1]);
}
bool Interval_equality(Interval interval,Interval another_interval)
{
    return interval.data[0]==another_interval.data[0] && interval.data[1]==another_interval.data[1];
}
bool Interval_difference(Interval interval,Interval another_interval)
{
    return interval.data[0]!=another_interval.data[0] || interval.data[1]!=another_interval.data[1];
}

#endif // INTERVAL_H_INCLUDED
