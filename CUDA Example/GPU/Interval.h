#ifndef INTERVAL_H_INCLUDED
#define INTERVAL_H_INCLUDED

struct CInterval
{
    float data[2];
};

__device__ CInterval Interval_create(float left,float right)
{
    CInterval newInterval;
    newInterval.data[left>right]=left;
    newInterval.data[left<=right]=right;
    return newInterval;
}
__device__ float Interval_get_mid_point(CInterval interval)
{
    return (interval.data[0]+interval.data[1])/2.0;
}
__device__ bool Interval_contains(CInterval interval,float point)
{
    return interval.data[0]<=point && point<=interval.data[1];
}
__device__ bool Interval_contains(CInterval interval,CInterval another_interval)
{
    return interval.data[0]<=another_interval.data[0] && another_interval.data[1]<=interval.data[1];
}
__device__ bool Interval_intersects(CInterval interval,CInterval another_interval)
{
    float ini_point=max(interval.data[0],another_interval.data[0]);
    float end_point=min(interval.data[1],another_interval.data[1]);
    return ini_point<=end_point;
}
__device__ bool Interval_is_left_of(CInterval interval,float point)
{
    return interval.data[1]<point;
}
__device__ bool Interval_is_right_of(CInterval interval,float point)
{
    return point<interval.data[0];
}
__device__ bool Interval_is_left_of(CInterval interval,CInterval another_interval)
{
    return Interval_is_left_of(interval,another_interval.data[0]);
}
__device__ bool Interval_is_right_of(CInterval interval,CInterval another_interval)
{
    return Interval_is_right_of(interval,another_interval.data[1]);
}
__device__ bool Interval_equality(CInterval interval,CInterval another_interval)
{
    return interval.data[0]==another_interval.data[0] && interval.data[1]==another_interval.data[1];
}
__device__ bool Interval_difference(CInterval interval,CInterval another_interval)
{
    return interval.data[0]!=another_interval.data[0] || interval.data[1]!=another_interval.data[1];
}

#endif // INTERVAL_H_INCLUDED
