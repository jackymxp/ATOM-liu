#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#define ATOM_SIZE       10000000
#define HEAD_SIZE       9

#define SHOW_PROCESSING    1

#if SHOW_PROCESSING
typedef struct
{
    unsigned int hasFinished;
    pthread_mutex_t mute;
}DEBUGPARA;

DEBUGPARA DebugPara;
#endif

typedef struct
{
    unsigned int id;
    unsigned int type;
    double x, y, z;
    double c_ke;
    double ave_ke;
}ATOM;

typedef struct
{
    ATOM atom_vec[ATOM_SIZE];
    unsigned int atom_size;
    char head_info[HEAD_SIZE][100];
}ATOMPARA;

ATOMPARA ATOMVECT;

static inline ATOM line2ATOM(const char* line)
{
    ATOM atom;
    sscanf(line, "%d %d %lf %lf %lf %lf", &atom.id, &atom.type, &atom.x, &atom.y, &atom.z, &atom.c_ke);
    return atom;
}

static inline void ATOM2line(const ATOM* atom, char* line)
{
    sprintf(line, "%d %d %.8g %.8g %.8g %.8g %.8g\n",atom->id, atom->type, atom->x, atom->y, atom->z, atom->c_ke, atom->ave_ke);
}

static inline double distanceOfATOM(const ATOM* a, const  ATOM* b)
{
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    double dz = a->z - b->z;
    double dis = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);
    return dis;
}

#define FULL_BAR_SPACE   80
static inline void printProcessBar(double percent)
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

//对[start, end)区间的原子进行处理
static void dealSingleATOM(int start, int end)
{
    unsigned int len = ATOMVECT.atom_size;
    for(int i = start; i < end; i++)
    {
        ATOM* atom = &ATOMVECT.atom_vec[i];
        double sumOfATOM = 1;
        double sumOfc_ke = atom->c_ke;
        for(int left = i - 1; left >= 0; left--)
        {
            ATOM* tmp = &ATOMVECT.atom_vec[left];
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
            ATOM* tmp = &ATOMVECT.atom_vec[right];
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
/********************************************************************************************************/

static inline int readFiles(const char* filepath)
{
    FILE* fp = NULL;
    if((fp= fopen(filepath, "r")) == NULL)
    {
        printf("open file failed, please check %s is not existed! \n", filepath);
        return -1;
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
        ATOM at = line2ATOM(line);
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
        return -1;
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
    return ((ATOM*)a)->x - ((ATOM*)b)->x;
}

static inline int compid(const void* a, const void* b){
    return ((ATOM*)a)->id - ((ATOM*)b)->id;
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


