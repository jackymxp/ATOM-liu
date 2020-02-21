#ifndef STRESS_H
#define STRESS_H

#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#define ATOM_SIZE       1000000
#define HEAD_SIZE       9

#define SHOW_PROCESSING 1
#define FULL_BAR_SPACE   80

struct ATOM{
    unsigned int id;
    unsigned int type;
    double x, y, z;
    double cs1, cs2, cs3, cs4, cs5, cs6;
    double c_von;
    double dst;
};


#if SHOW_PROCESSING
struct DEBUGPARA
{
    unsigned int hasFinished;
    pthread_mutex_t mute;
};
extern struct DEBUGPARA DebugPara;
#endif

static inline struct ATOM line2ATOM(const char* line)
{
    struct ATOM atom;
    sscanf(line, "%d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &atom.id, &atom.type, &atom.x, &atom.y, &atom.z, &atom.cs1, &atom.cs2, &atom.cs3, &atom.cs4, &atom.cs5, &atom.cs6, &atom.c_von);
    return atom;
}

void ATOM2line(const struct ATOM* atom, char* line)
{
    sprintf(line, "%d %d %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g\n",atom->id, atom->type, atom->x, atom->y, atom->z, atom->cs1, atom->cs2, atom->cs3, atom->cs4, atom->cs5, atom->cs6, atom->c_von, atom->dst);
}


//对[start, end)区间的原子进行处理
void dealSingleATOM(int start, int end)
{
    unsigned int len = getATOMSize();
    for(int i = start; i < end; i++)
    {
        struct ATOM* atom = NULL;
        struct ATOM* tmp = NULL;
        atom = getIndexATOM(i);
        double sumOfATOM = 1;
        double sumOfc_ke = atom->cs1;
        for(int left = i - 1; left >= 0; left--)
        {
            tmp = getIndexATOM(left);
            if(fabs(tmp->y - atom->y) > 10 || fabs(tmp->z - atom->z) > 10)
                continue;
            if(fabs(tmp->x - atom->x) > 10)
                break;
            if(distanceOfATOM(tmp, atom) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp->cs1;
            }
        }
        for(int right = i + 1; right < len; right++)
        {
            tmp = getIndexATOM(right);
            if(fabs(tmp->y - atom->y) > 10 || fabs(tmp->z - atom->z) > 10)
                continue;
            if(fabs(tmp->x - atom->x) > 10)
                break;
            if(distanceOfATOM(tmp, atom) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp->cs1;
            }
        }
        atom->dst = sumOfc_ke / sumOfATOM;

#if SHOW_PROCESSING
        pthread_mutex_lock(&DebugPara.mute);
        DebugPara.hasFinished++;
        if(DebugPara.hasFinished % 1000 == 0)
        {
            float percent = (float)DebugPara.hasFinished / len;
            printProcessBar(percent);
        }
        pthread_mutex_unlock(&DebugPara.mute);
#endif
    }
}

#endif /* STRESS_H */
