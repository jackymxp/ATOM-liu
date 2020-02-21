#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <thread>
#include <atomic>

using namespace std;

const unsigned int HEAD_SIZE = 9;

struct ATOM
{
    unsigned int id;
    unsigned int type;
    double x, y, z;
    double c_ke;
    double ave_ke;
    ATOM(){}
    ATOM(unsigned int id, unsigned int type, double x, double y, double z, double c_ke) : id(id), type(type), x(x), y(y), z(z), c_ke(c_ke){}
    friend ostream& operator<<(ostream& os, const ATOM& a);
    friend istream& operator<<(istream& is, ATOM& a);
};
ostream& operator<<(ostream& os, const ATOM& a)
{
    os << a.id << " " << a.type << " " << a.x << " " << a.y << " " << a.z << " " << a.c_ke << " " << a.ave_ke;
    return os;
}

istream& operator>>(istream& is, ATOM& a)
{
    is >> a.id >> a.type >> a.x >> a.y >> a.z >> a.c_ke;
    return is;
}

vector<string> HeadInfo;
vector<ATOM> ATOMVECT;

void readFiles(const string& filepath)
{
    ifstream infile(filepath);
    assert(infile.is_open());
    string line;
    for(int i = 0; i < HEAD_SIZE; i++)
    {
        getline(infile, line);
        HeadInfo.emplace_back(line);
    }
    while(getline(infile, line))
    {
        stringstream sstream(line);
        ATOM atom;
        try{
            sstream >> atom;
            ATOMVECT.emplace_back(atom);
        }
        catch(exception& p){
            cout << line << " " << p.what() << endl;
            abort();
        }

    }
    infile.close();
}

void writeFiles(const string& filepath)
{
    ofstream outfile(filepath);
    assert(outfile.is_open());
    string line;
    for(auto & i : HeadInfo)
        outfile << i << endl;
    for(auto & i : ATOMVECT)
        outfile << i << endl;
    outfile.close();
}


static inline double distanceOfATOM(const ATOM& a, const ATOM& b)
{
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    double dis = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);
    return dis;
}

atomic_long hasFinished(0);

#define FULL_BAR_SPACE   80
static inline void printProcessBar(double percent)
{
    printf("[");
    int tmp = percent * FULL_BAR_SPACE;
    for(int q = 0; q < FULL_BAR_SPACE; q++)
    {
        if(q < tmp)
            printf("\033[32;47m#\033[0m");
        else if(q == tmp)
            printf(">");
        else if(q > tmp)
            printf(" ");
    }
    cout << "] " << hasFinished << " \033[32;47m" << 100 * percent << "%\033[0m" << "\r" ;
}


//对[start, end)区间的原子进行处理
void dealSingleATOM(int start, int end)
{
    size_t len = ATOMVECT.size();
    for(int i = start; i < end; i++)
    {
        ATOM& atom = ATOMVECT[i];
        double sumOfATOM = 1;
        double sumOfc_ke = atom.c_ke;
        for(int left = i - 1; left >= 0; left--)
        {
            ATOM& tmp = ATOMVECT[left];
            if(fabs(atom.y - tmp.y) > 10 || fabs(atom.z - tmp.z) > 10)  continue;
            if(fabs(atom.x - tmp.x) > 10)   break;
            if(distanceOfATOM(atom, tmp) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp.c_ke;
            }
        }

        for(int right = i + 1; right < len; right++)
        {
            ATOM& tmp = ATOMVECT[right];
            if(fabs(atom.y - tmp.y) > 10 || fabs(atom.z - tmp.z) > 10)  continue;
            if(fabs(atom.x - tmp.x) > 10)   break;
            if(distanceOfATOM(atom, tmp) <=  100)
            {
                sumOfATOM += 1;
                sumOfc_ke += tmp.c_ke;
            }
        }
        atom.ave_ke = sumOfc_ke / sumOfATOM;
        hasFinished++;
        if(hasFinished % 1000 == 0)
        {
            printProcessBar((double)hasFinished / len);
        }
    }
}

void threadCoreCpp(int left, int right)
{
    dealSingleATOM(left, right);
}

void dealATOMFiles(const string& infilepath, const string& outfilepath)
{
    cout << "start read " << infilepath << endl;
    readFiles(infilepath);
    cout << "finish read " << infilepath << "start sort files" << endl;
    size_t len = ATOMVECT.size();
    cout << "start deal " << infilepath << "   the size : " << ATOMVECT.size() << endl;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    sort(ATOMVECT.begin(), ATOMVECT.end(), [](const ATOM& a, const ATOM& b){return a.x < b.x;});
    cout << "finish sort " << infilepath << endl;
    unsigned long const num_thread = std::thread::hardware_concurrency();   //获取当前CPU核心数量
    unsigned int const step = len / num_thread;
    unsigned int const numOfStep = num_thread - len % num_thread;
    vector<thread> thr;
    for(unsigned int i = 0; i < numOfStep; i++)
        thr.push_back(thread(threadCoreCpp, i*step, (i+1)*step));
    for(unsigned int i = 0; i < len % num_thread; i++)
        thr.push_back(thread(threadCoreCpp, numOfStep*step + i*(step+1), numOfStep*step + (i+1)*(step+1)));
    for(auto & t : thr)
        t.join();

    sort(ATOMVECT.begin(), ATOMVECT.end(), [](const ATOM& a, const ATOM& b){return a.id < b.id;});
    writeFiles(outfilepath);
    cout << " deal " << infilepath << " finished ~~~" << endl;

    gettimeofday(&end, NULL);
    double duration = ((end.tv_sec - start.tv_sec) * pow(10, 6) + (end.tv_usec - start.tv_usec)) / pow(10, 6);
    cout << "deal " << infilepath << "  has findished, it spent " << duration << endl;
}

int main(int argc, char* argv[])
{
    const string infilepath(argv[1]);	
    const string outfilepath(argv[2]);	
    dealATOMFiles(infilepath, outfilepath);

    return 0;
}
