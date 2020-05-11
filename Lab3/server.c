#include "lab3header.h"


#define UDP_PORT_1 8888
#define UDP_PORT_2 8889
#define BUFFERLEN 1024

#define IP_HEADER 60 //max ip header in bytes 
#define UDP_HEADER 8
#define ETHERNET_FRAME 26
struct uheader* buffer_to_header(uint8_t* buffer){
        struct uheader* hptr = (struct uheader*)buffer;
        hptr->seq_number = ntohl(hptr->seq_number);
        return hptr;
}


int usock[2];
int client_available[2] = { 1 , 1 };
pthread_t uthread[2];
unsigned int totalrecv[2];
unsigned int packet_counter[2];
unsigned long long jitters[2] = { 0 , 0 };
unsigned long long sdjitter[2]={0,0};
struct timespec start_time[2];
struct timespec end_time[2];


static inline void
print_statistics(unsigned int received, struct timespec start, struct timespec end)
{
    double elapsed = end.tv_sec - start.tv_sec
                     + (end.tv_nsec - start.tv_nsec) * 1e-9;
    double megabytes = (double)received / 1000.0;
    printf("Data received: %lf MB\n", megabytes);
    printf("Transfer time: %lf seconds\n", elapsed);
    printf("Goodput achieved: %lf MB/s\n", megabytes / elapsed);
}

void die(char *s)
{
    perror(s);
    exit(1);
}


int TCPsocket(){
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
        perror("opening TCP socket");
        exit(EXIT_FAILURE);
    }
    return sock;
}

int UDPsocket(){
    int sock;
    if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        perror("socket");
        exit(0);
    }
    return sock;
}

void* UDPservice(void* clnt){
    int usock = ((struct u_args*)clnt)->usock;
    int cl_no = ((struct u_args*)clnt)->cl_no;
    struct sockaddr_in  sin_other;
    int buf_len = 2056;
    int pre , i, slen = sizeof(sin_other) , recv_len;
    int timeout = 15000;
    char buf[BUFFERLEN];
    struct pollfd fds[1];

    fds[0].fd = usock;
    fds[0].events = 0;
    fds[0].events |= POLLIN;

    while( ( poll(fds , 1 , timeout)) == 0 ){
        printf("Waiting for data...\n");
    }
    clock_gettime(CLOCK_REALTIME, &start_time[cl_no]);
    packet_counter[cl_no] = 0;
    totalrecv[cl_no] = 0;
    struct timespec tmptimejitter;	//tmptime=0
    tmptimejitter.tv_sec=0;
    tmptimejitter.tv_nsec=0;
    struct timespec timejitter;
    int TEMPcounter=0;
    unsigned long long elapsed;
    jitters[cl_no]=0;
    sdjitter[cl_no]=0;
    while(1)
    {
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(usock, buf, BUFFERLEN, 0, (struct sockaddr *) &sin_other, &slen)) == -1)
        {
            //die("recvfrom()");
            printf("recvfrom fail\n");
            break;
        }
                //clock get time time
        clock_gettime(CLOCK_MONOTONIC_RAW, &timejitter);
	if(packet_counter[cl_no]!=0){
		elapsed = ((timejitter.tv_sec - tmptimejitter.tv_sec )*1e+9)
                     + (timejitter.tv_nsec - tmptimejitter.tv_nsec);
		TEMPcounter++;
               	jitters[cl_no]+=elapsed;
		sdjitter[cl_no]+=elapsed*elapsed;
        }
	tmptimejitter.tv_sec=timejitter.tv_sec;
	tmptimejitter.tv_nsec=timejitter.tv_nsec;

        totalrecv[cl_no]+=recv_len;
        buf[recv_len]='\0';
        ++packet_counter[cl_no];
        clock_gettime(CLOCK_REALTIME, &end_time[cl_no]);
        struct uheader* uh = buffer_to_header(buf);
    }
    return NULL;
}


void* serviceClient(void* arg){
    struct service_args *serv = (struct service_args *)arg;
    int acc = serv->accepted;
    int cl_no = serv->cl_no;
    printf("Im ready to serveice client %d\n" , serv->cl_no);

    int  received;
    char buffer[BUFFERLEN];

    int usock = UDPsocket();
    struct sockaddr_in sin_me;
	
    // zero out the structure
    memset((char *) &sin_me, 0, sizeof(sin_me));


    sin_me.sin_family = AF_INET;
    if(cl_no==0){
        sin_me.sin_port = htons(UDP_PORT_1);
    }else if(cl_no==1){
        sin_me.sin_port = htons(UDP_PORT_2);
    }else{
        fprintf(stderr,"UDP::cannot accept more clients1\n");
        exit(0);
    }
    if(serv->ipaddr==NULL){
                sin_me.sin_addr.s_addr = INADDR_ANY;
    }else{
                sin_me.sin_addr.s_addr=inet_addr(serv->ipaddr);
    }

    //bind socket to port
    if( bind(usock , (struct sockaddr*)&sin_me, sizeof(sin_me) ) == -1)
    {
        die("bind");
    }

    struct u_args uarg;
    uarg.usock = usock;
    uarg.cl_no = cl_no;

    if(pthread_create( &uthread[cl_no]  , NULL, UDPservice , &uarg)!=0 ){
        die("pthread_create");
    }


    if(cl_no==0){
        if(send(acc, "8888" , 5, 0)!=5){
            die("send cl_no1");
        }
    }else if(cl_no==1){
        if(send(acc, "8889", 5, 0)!=5){
            die("send cl_no2");
        }
    }else{
        printf("client no error\n");
        exit(0);
    }
    struct uheader*  data;
    while( (received = recv(acc, buffer, BUFFERLEN, 0)>=0) ){
        printf("\nStop recived from client %d\n" ,cl_no );
        data = buffer_to_header(buffer);
        sleep(1);
	close(usock);
        if( pthread_cancel(uthread[cl_no])!=0){
            die("pthread_cancel:");
        }
        break;
    }
    client_available[cl_no] = 1;
    close(acc);
    printf("--------CLIENT%d------------\n" , cl_no);
    printf("The average jitter is %llu nanosec\n",jitters[cl_no]/(packet_counter[cl_no]-1));
    double SDforSQRT=(double) (sdjitter[cl_no]/(packet_counter[cl_no]-1));
     printf("The SD of jitter is %lf nanosec\n",sqrt(SDforSQRT));
    print_statistics(totalrecv[cl_no] , start_time[cl_no] , end_time[cl_no] );
    printf("packet loss percentage:%lf %%\n" , (100.0*((double)data->seq_number-((double)packet_counter[cl_no]-1)))/(double)data->seq_number);
    return NULL;
}


int iperf_server(char *ipstr, int port){
    int isListenerActive = 0;
    pthread_t tcpL_th;
    int tsock;

    tsock = TCPsocket();

    int accepted;
    struct service_args arg;
    char buffer[BUFFERLEN];

    struct sockaddr_in sin;
    struct sockaddr client_addr;
    socklen_t client_addr_len;
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if(ipstr==NULL){
		sin.sin_addr.s_addr = INADDR_ANY;
    }else{
		sin.sin_addr.s_addr=inet_addr(ipstr);
    }
    if(bind(tsock, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1){
        die("TCP bind");
    }

    if(listen(tsock, 1000) == -1){
        die("TCP listen");
    }
    /* Ok, a tricky part here. See man accept() for details */
    client_addr_len = sizeof(struct sockaddr);
    while( (accepted = accept(tsock, &client_addr, &client_addr_len)) > 0 ){
        if(!client_available[0]&&!client_available[1]){
            printf("cannot accept more than two clients\n");
            continue;
        }
        arg.accepted = accepted;
	arg.ipaddr=ipstr;
        if(client_available[0]){
            arg.cl_no = 0;
        }else{
            arg.cl_no = 1;
        }
        if(pthread_create( &tcpL_th   , NULL, serviceClient , &arg)!=0 ){
            die("pthread_create");
        }


        printf("New connection accepted!\n");
        client_available[arg.cl_no] = 0;
    }
}
