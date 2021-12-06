#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <deque>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>
#define TRACE_PERIOD (1)    //1
std::deque<int> trace;
char* dst_ip;
using namespace std;
void SetBandwidth(int);

void timer(int sig)
{
    if ( SIGALRM == sig )
    {
	SetBandwidth(trace.front());
        trace.pop_front();
 	alarm(TRACE_PERIOD);
    }
}

void SetBandwidth(int band)
{
    ofstream ofile;
    ofile.open("BandwidthLog.txt",ios::out|ios::app);
    char command[256] = {0};
    //char command1[256] = {0};
    //system("sudo tcdel --device enp9s0 --all");
    struct timeval tv;
    gettimeofday(&tv,NULL);
    struct tm* pTime;
    pTime = localtime(&tv.tv_sec);
    char sTemp[30] = {0};
    snprintf(sTemp, sizeof(sTemp), "%04d-%02d-%02d %02d:%02d:%02d %03d:%03d", pTime->tm_year+1900, \
           pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, \
           tv.tv_usec/1000,tv.tv_usec%1000);
    ofile<<(string)sTemp<<"\t"<<band<<endl;
    sprintf(command, "sudo tcset --device enp9s0 --rate %dM --delay 10ms --loss 0.02 --network %s --overwrite",band, dst_ip);
    //sprintf(command1, "sudo tcset --device enp9s0 --direction outgoing --rate %dM --delay 10ms  --network %s --overwrite",band, dst_ip);
    //
    std::cout << command << std::endl;
    //std::cout << command1 << std::endl;
    system(command);
    //system(command1);
    system("tcshow --device enp9s0");
}

void StopShape()
{
  system("sudo tcdel --device enp9s0 --all");
}

int main(int argc, char* argv[])
{
    if ( argc < 3 )
    {
 	printf("sudo ./TrafficControl dst_ip trace_file\n");
 	return -1;
    }
    printf("%s %s\n", argv[1], argv[2]);

    std::ofstream file;
    string str_file;
    str_file="BandwidthLog.txt";
    file.open(str_file);
    file.close();

    dst_ip = argv[1];
    char buffer[256] = {0};
    std::ifstream is(argv[2]);
    if ( !is.good() )  
       return -1;
    while ( !is.eof() )
    {
	is.getline(buffer, 256);
 	trace.push_back(atoi(buffer));      
    }
   
    signal(SIGALRM, timer);
    alarm(TRACE_PERIOD);
    getchar();
    StopShape(); 
    return 0;
}
