#ifndef AVE_H
#define AVE_H

#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#define ATOM_SIZE       10000000
#define HEAD_SIZE       9

#define SHOW_PROCESSING 1
#define FULL_BAR_SPACE   80

struct ATOM
{
    unsigned int id;
    unsigned int type;
    double x, y, z;
    double c_ke;
    double ave_ke;
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
    sscanf(line, "%d %d %lf %lf %lf %lf", &atom.id, &atom.type, &atom.x, &atom.y, &atom.z, &atom.c_ke);
    return atom;
}

static inline void ATOM2line(const struct ATOM* atom, char* line)
{
    sprintf(line, "%d %d %.8g %.8g %.8g %.8g %.8g\n",atom->id, atom->type, atom->x, atom->y, atom->z, atom->c_ke, atom->ave_ke);
}


//对[start, end)区间的原子进行处理
static void dealSingleATOM(int start, int end)
{
    unsigned int len = getATOMSize();
    for(int i = start; i < end; i++)
    {
        struct ATOM* atom = NULL;
        struct ATOM* tmp = NULL;
        atom = getIndexATOM(i);
        double sumOfATOM = 1;
        double sumOfc_ke = atom->c_ke;
        for(int left = i - 1; left >= 0; left--)
        {
            tmp = getIndexATOM(left);
            if(fabs(tmp->y - atom->y) > 10 || fabs(tmp->z - atom->z) > 10){continue;}
            if(fabs(tmp->x - atom->x) > 10) break;
            if(distanceOfATOM(tmp, atom) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp->c_ke;
            }
        }

        for(int right = i + 1; right < len; right++)
        {
            tmp = getIndexATOM(right);
            if(fabs(tmp->y - atom->y) > 10 || fabs(tmp->z - atom->z) > 10){continue;}
            if(fabs(tmp->x - atom->x) > 10)     break;
            if(distanceOfATOM(tmp, atom) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp->c_ke;
            }
        }
        atom->ave_ke = sumOfc_ke / sumOfATOM;
#if SHOW_PROCESSING
        pthread_mutex_lock(&DebugPara.mute);
        DebugPara.hasFinished++;
        if(DebugPara.hasFinished % 1000 == 0)	
        {
            float percent = (float)DebugPara.hasFinished / len;
#if 0
            printf("tatal_size = %8u   ###   %8u <<<---->>> %lf %%\n",len,  DebugPara.hasFinished, percent);
#else
            printProcessBar(percent);
#endif
        }
        pthread_mutex_unlock(&DebugPara.mute);
#endif
    }
}
#endif /* AVE_H */
