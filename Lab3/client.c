#include "lab3header.h"

uint8_t* header_to_buffer(struct uheader* hd , uint8_t* buffer){
  hd->seq_number=htonl(hd->seq_number);
  memcpy(buffer , hd , sizeof(struct uheader));
  return buffer+sizeof(struct uheader);
}


int iperf_client(char *ipstr,int port,int packet_size,int bandwidth,int parallelstreams,int duration,int one_way_only){
  uint8_t buffer[BUFF_SIZE];
  int tsock , i;
  uint32_t seqNum=0;
  struct uheader *seqNumStruct=malloc(sizeof(struct uheader));
  /*For while and sleep*/
  struct timespec tim, tim2;
  time_t endtime;
  time_t now = time(NULL);
  float sec2nano=1000000000;
  float mbandwidth=(float)bandwidth/8.0;
  int seconds2sleep=packet_size/mbandwidth;
  float sleeptime=(float)(packet_size*sec2nano/mbandwidth)-(seconds2sleep*sec2nano);  //sec to nanoseconds
  tim.tv_sec = seconds2sleep;
  tim.tv_nsec = (long)sleeptime; //1ms
  struct timespec m_start_time;


  struct sockaddr *tclient_addr;
  socklen_t tclient_addr_len;
  if((tsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1){
    perror("Opening TCP socket");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in tsin;
  memset(&tsin, 0, sizeof(struct sockaddr_in));
  tsin.sin_family = AF_INET;
  /*Port that server listens at */
  tsin.sin_port = htons(port);
  /* The server's IP*/
  tsin.sin_addr.s_addr = inet_addr(ipstr);
  if(connect(tsock, (struct sockaddr *)&tsin, sizeof(struct sockaddr_in)) == -1){
    perror("TCP connect");
    exit(EXIT_FAILURE);
  }

  /*Waiting for Server to open UDP socket*/

  //to do
  //create udp socket
  int usock, portNum, nBytes;
  struct sockaddr_in serverAddr;
  socklen_t addr_size;
  int udpPORT=0;
  int received=0;
  if((received = recv(tsock, buffer, BUFF_SIZE-1, 0)) <= 0){
      perror("Received Error");
      exit(-1);
  }
  buffer[received] = '\0';
  
  udpPORT=atoi(buffer);

  usock = socket(PF_INET, SOCK_DGRAM, 0);       //FROM SERVER

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(udpPORT);         //FROM SERVER
  serverAddr.sin_addr.s_addr = inet_addr(ipstr);
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  //create packets
  //sent packets
  addr_size = sizeof serverAddr;
  now = time(NULL);
  endtime = now + duration;
  if(duration==0){
    endtime = now + DAY_SECS; //a day
  }

  int bufferlength = packet_size; // - UDP_header;    //8 byte UDP Header
  int startflag=0;
  while (now < endtime){
    //create packet
    seqNumStruct->seq_number=seqNum;
    seqNum++; //updating seq number          
    header_to_buffer(seqNumStruct ,buffer);
    if(startflag==0){
      clock_gettime(CLOCK_REALTIME, &m_start_time);
      startflag=1;
    }
    sendto(usock,buffer,bufferlength,0,(struct sockaddr *)&serverAddr,addr_size);

    nanosleep(&tim,&tim2);
	now = time(NULL);
  }
  struct uheader *tosend=malloc(sizeof(struct uheader));

  tosend->seq_number=seqNum-1;
  header_to_buffer(tosend, buffer);
  
  bufferlength=sizeof(buffer);
  if(send(tsock, buffer , bufferlength, 0)!=bufferlength){
    perror("closing connection"); //close(tsock);
  }
  sleep(1);	//FEEETA
  close(tsock); 
  close(usock);
  printf("\nUDP experiment is over!\n");
  return 1;
}

