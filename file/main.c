#include "main.h"
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

struct ATOMPARA
{
    struct ATOM atom_vec[ATOM_SIZE];
    unsigned int atom_size;
    char head_info[HEAD_SIZE][100];
};

struct ATOMPARA ATOMVECT;

inline struct ATOM* getIndexATOM(unsigned int index)
{
    return &(ATOMVECT.atom_vec[index]);
}


inline unsigned int getATOMSize(void)
{
    return ATOMVECT.atom_size;
}

inline void printProcessBar(double percent)
{
    printf("\033[1A"); //先回到上一行
    printf("[");
    for(int q = 0; q < FULL_BAR_SPACE; q++)
    {
        if(q < percent * FULL_BAR_SPACE)
            printf("\033[32;47m#\033[0m");
        else if(q == percent)
            printf(">");
        else if(q > percent)
            printf(" ");
    }
    printf("] %d <<>> \033[32;47m%.2lf%%\033[0m\n",DebugPara.hasFinished, 100 * percent);
}


inline double distanceOfATOM(const struct ATOM* a, const struct ATOM* b)
{
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    double dz = a->z - b->z;
    double dis = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);
    return dis;
}

#if SHOW_PROCESSING
struct DEBUGPARA DebugPara;
#endif

static inline int readFiles(const char* filepath)
{
    FILE* fp = NULL;
    if((fp= fopen(filepath, "r")) == NULL)
    {
        printf("open file failed, please check %s is not existed! \n", filepath);
        exit(1);
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t readByte;
    int i = 0;
    for(i = 0; i < HEAD_SIZE; i++)
    {
        readByte = getline(&line, &len, fp);
        strcpy(ATOMVECT.head_info[i], line);
    }
    i = 0;
    while ((readByte = getline(&line, &len, fp)) != -1)
    {
        assert(i < ATOM_SIZE);
        struct ATOM at = line2ATOM(line);
        ATOMVECT.atom_vec[i++] = at;
    }
    ATOMVECT.atom_size = i;
    fclose(fp);
    return 0;
}

static inline int writeFiles(const char* filepath)
{
    FILE* fp = NULL;
    if((fp= fopen(filepath, "w+")) == NULL)
    {
        printf("open file failed, please check %s is not existed! \n", filepath);
        exit(1);
    }
    for(int i = 0; i < HEAD_SIZE; i++)
        fprintf(fp, "%s", ATOMVECT.head_info[i]);

    char line[200];
    for(int i = 0; i < ATOMVECT.atom_size; i++)
    {
        ATOM2line(&ATOMVECT.atom_vec[i], line);
        fprintf(fp, "%s", line);
    }
    fclose(fp);
    return 0;
}

static inline int compx(const void* a, const void* b){
    return ((struct ATOM*)a)->x - ((struct ATOM*)b)->x;
}

static inline int compid(const void* a, const void* b){
    return ((struct ATOM*)a)->id - ((struct ATOM*)b)->id;
}

struct args
{
    unsigned int left;
    unsigned int right;
};

void* threadCore(void* args)
{
    struct args* p = (struct args*)args;
    unsigned int left = p->left;
    unsigned int right = p->right;
    dealSingleATOM(left, right);
    return NULL;
}

void dealATOMFiles(const char* infilepath, const char* outfilepath)
{
#if SHOW_PROCESSING	
    pthread_mutex_init(&DebugPara.mute, NULL);
#endif
    readFiles(infilepath);
#if 1
    struct timeval start, end;
    gettimeofday(&start, NULL);
    unsigned int len = ATOMVECT.atom_size;
    printf("start deal %s, the size = %u\n",infilepath, ATOMVECT.atom_size);
    //按照x坐标进行排序
    qsort(ATOMVECT.atom_vec, ATOMVECT.atom_size, sizeof(ATOMVECT.atom_vec[0]), compx);

    unsigned int const num_threads = sysconf(_SC_NPROCESSORS_CONF);   //获取当前CPU核心数量
    printf("current cpu num_threads = %d\n\n", num_threads);
    unsigned int const step = len / num_threads;
    unsigned int const halfStep = len % num_threads;
    unsigned int const numOfStep = num_threads - halfStep;
    struct args* arg = (struct args*)malloc(sizeof(struct args) * num_threads);
    for(unsigned int i = 0; i < numOfStep; i++)
    {
        arg[i].left = i * step;
        arg[i].right = (i+1)*step;
    }
    for(unsigned int i = 0; i < halfStep; i++)
    {
        arg[i+numOfStep].left = numOfStep * step + i * (step+1);
        arg[i+numOfStep].right = numOfStep * step + (i+1)*(step+1);
    }
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);
    for(unsigned int i = 0; i < num_threads; i++)
    {
        int ret = pthread_create(&tid[i], NULL, threadCore, &arg[i]);
        if(ret != 0)
            printf("pthread_create error:error_code %d", ret);
    }

    for(unsigned int i = 0; i < num_threads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    free(tid);
    free(arg);

    qsort(ATOMVECT.atom_vec, ATOMVECT.atom_size, sizeof(ATOMVECT.atom_vec[0]), compid);

    writeFiles(outfilepath);

#if SHOW_PROCESSING
    pthread_mutex_destroy(&DebugPara.mute);
#endif
    gettimeofday(&end, NULL);
    double duration = ((end.tv_sec - start.tv_sec) * pow(10, 6) + (end.tv_usec - start.tv_usec)) / pow(10, 6);
    printf("deal %s has findished, it spent %f s\n", infilepath, duration);
#endif
}

int main(int argc, char* argv[])
{
    char* infilepath = argv[1];
    char* outfilepath = argv[2];
    dealATOMFiles(infilepath, outfilepath);
    return 0;
}



