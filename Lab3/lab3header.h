#include <inttypes.h> //checksum print
#include <assert.h>       //assert
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <poll.h>
#include <math.h>

#define UDP_header 8
#define BUFF_SIZE 1024
#define DAY_SECS 86400

struct service_args{
    int accepted;
    int cl_no;
    char* ipaddr;
};

struct u_args{
    int usock;
    int cl_no;
};

struct client{
    pthread_t uthread;
    int tsock , usock;
    short int cl_no;
};


struct uheader{
    uint32_t seq_number;
};

int iperf_client(char *ipstr,int port,int packet_size,int bandwidth,int parallelstreams,int duration,int one_way_only);

int iperf_server(char *ipstr,int port);
