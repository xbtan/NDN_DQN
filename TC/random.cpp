#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

int main()
{
    fstream ofile;
    ofile.open("TrafficSetData2.txt");
    int count = 40;
    while(count > 0)
    {   
       srand((unsigned)time(NULL));
       int res =  rand()%70 + 60 ;   //200-400
       ofile<<res<<endl;
       float secs = 1.0;
       clock_t delay;
       delay = secs*CLOCKS_PER_SEC;
       clock_t start = clock();
       while(clock()-start<delay);
       count--;
    }
    ofile.close(); 
    return 0;
}
