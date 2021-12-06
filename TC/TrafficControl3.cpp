#include <iostream>
#include <fstream>
#include <deque>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fstream>

#include <unistd.h>

std::deque<int> trace;
char* dst_ip;
using namespace std;
void SetBandwidth(int);

void Enter()
{
    SetBandwidth(trace.front());
    trace.pop_front();
}

void SetBandwidth(int band)
{
    char command[256] = {0};
 
    sprintf(command, "sudo tcset --device enp9s0 --rate %dM --delay 10ms --loss 0.02 --network %s --overwrite",band, dst_ip);
    //sprintf(command1, "sudo tcset --device enp9s0 --direction outgoing --rate %dM --delay 10ms  --network %s --overwrite",band, dst_ip);
    //
    std::cout << command << std::endl;
    //std::cout << command1 << std::endl;
    system(command);
    //system(command1);
    //system("tcshow --device enp9s0");
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

    while (1)
    {
        Enter();
        //cin.get();

        usleep(100000);    //us
    }
   
    //signal(SIGALRM, timer);
    //alarm(TRACE_PERIOD);
    //getchar();
    //opShape(); 
    return 0;
}
