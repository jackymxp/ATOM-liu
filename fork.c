#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
#include <assert.h>
#include <math.h>

typedef struct
{
    char dir_file[100][256];
    size_t size;
}DirFiles;

/* Show all files under dir_name , do not show directories ! */
static DirFiles showAllFiles(const char* dir_name)
{
    assert(dir_name != NULL);
    struct stat s;
    lstat(dir_name,&s);
    if(!S_ISDIR( s.st_mode ) )
    {
        perror("dir_name is not a valid directory !\n");
        exit(1);
    }

    struct dirent * filename;    // return value for readdir()
    DIR * dir;                   // return value for opendir()
    dir = opendir( dir_name );
    if( NULL == dir )
    {
        printf("can not open  %s ",dir_name);
        exit(1);
    }
    printf("successfully opened %s!\n",dir_name);
    int i = 0;
    DirFiles files;
    while( ( filename = readdir(dir) ) != NULL )
    {
        if( strcmp( filename->d_name , "." ) == 0 || 
                strcmp( filename->d_name , "..") == 0)
            continue;
        strcpy(files.dir_file[i], filename->d_name);
        i++;
    }
    files.size = i;
    return files;
}


DirFiles findFiles(const char* str)
{
    return showAllFiles(str);
}


int main(int argc, char* argv[])
{
    char* app = argv[1];
    char* dir_name = argv[2];

    DirFiles res = findFiles(dir_name); 

    size_t len = res.size;
    for(int i = 0; i < len; i++)
        printf("%s\n", res.dir_file[i]);
    printf("size = %zu\n", res.size);

    struct timeval start, end;
    gettimeofday(&start, NULL);

    pid_t pid;
    int i = 0;
    for(i = 0; i < len; i++)
    {
        pid = fork();
        if(pid == 0 || pid == -1)
            break;
    }
    // child process 
    if(i != len)
    {
        char para[100];
        char dst[100];
        sprintf(para, "%s/%s", dir_name, res.dir_file[i]);
        sprintf(dst, "%s===dst", para);
        execl(app, "ave", para, dst, NULL);
    }
    //parent process
    else
    {
        int status;
        pid_t wt;
        while((wt = wait(&status)) > 0);
        printf("finished all files  deal \n");
        gettimeofday(&end, NULL);
        double duration = ((end.tv_sec - start.tv_sec) * pow(10, 6) + (end.tv_usec - start.tv_usec)) / pow(10, 6);
        printf("deal %s dir has findished, include %zu files. it spent %f s\n", dir_name, len, duration);
    }
    assert(i <= len );
    return 0;
}
