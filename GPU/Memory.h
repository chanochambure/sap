#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#define NULL 0

struct RAM
{
    char* space;
    unsigned int startByte;
    unsigned int ramBytes;
    unsigned int totalBytes;
};

unsigned int RAM_getTotalBytes(unsigned int ramBytes)
{
    return ramBytes+ceil(ramBytes/8.0);
}

struct RAM RAM_create(char* space,unsigned int ramBytes)
{
    struct RAM newRam;
    newRam.space=NULL;
    newRam.ramBytes=newRam.startByte=newRam.totalBytes=0;
    if(ramBytes>0)
    {
        newRam.space=space;
        newRam.ramBytes=ramBytes;
        newRam.totalBytes=RAM_getTotalBytes(ramBytes);
        for(unsigned int i=0;i<newRam.totalBytes;++i)
            newRam.space[i]=0;
        newRam.startByte=newRam.totalBytes-newRam.ramBytes;
    }
    return newRam;
}

char* RAM_getDataByIndex(struct RAM* ram,unsigned int index)
{
    return &(ram->space[ram->startByte+index]);
}

unsigned int RAM_getIndexByData(struct RAM* ram,char* data)
{
    return (unsigned int)(data-ram->space)-ram->startByte;
}

char* RAM_allocate(struct RAM* ram,unsigned int bytes)
{
    if(bytes)
    {
        for(unsigned int i=0;i<ram->ramBytes;++i)
        {
            unsigned int j=0;
            while(j<bytes)
            {
                unsigned int ByteId=(i+j)/8;
                unsigned int bitId=(i+j)%8;
                if(ram->space[ByteId] & 1<<bitId)
                    break;
                ++j;
            }
            if(j==bytes)
            {
                for(unsigned int k=0;k<bytes;++k)
                {
                    unsigned int ByteId=(i+k)/8;
                    unsigned int bitId=(i+k)%8;
                    ram->space[ByteId] |= 1<<bitId;
                }
                return RAM_getDataByIndex(ram,i);
            }
        }
    }
    return NULL;
}

void RAM_free(struct RAM* ram,char* data,unsigned int bytes)
{
    unsigned int index=RAM_getIndexByData(ram,data);
    for(unsigned int i=0;i<bytes;++i)
    {
        unsigned int ByteId=(index+i)/8;
        unsigned int bitId=(index+i)%8;
        ram->space[ByteId] &= ~(1<<bitId);
    }
}

#endif // MEMORY_H_INCLUDED
