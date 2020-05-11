#include "lab3header.h"

int
main(int argc, char **argv)
{
	int 		opt;
	int 		port=5001;		//DEFAULT
	int			exit_code = 0;
	char		*ipstr = NULL;
	uint8_t		is_server = 0;
	uint8_t		is_client = 0;
	uint8_t		one_way_only = 0;
	int packet_size = 0;
	int bandwidth = 0;
	int parallelstreams = 0;
	int duration = 0;

	int i;
	for(i=0;i<argc;i++){
		if(strcmp(argv[i],"-c")==0){
			is_client=1;
		}
		else if(strcmp(argv[i],"-s")==0){
			is_server=1;
		}
		else if(strcmp(argv[i],"-p")==0){
			port = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-a")==0){
			ipstr = strdup(argv[i+1]);
		}
		else if(strcmp(argv[i],"-l")==0){
			packet_size = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-b")==0){
			bandwidth = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-n")==0){
			parallelstreams = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-t")==0){
			duration = atoi(argv[i+1]);
		}
		else if(strcmp(argv[i],"-d")==0){
			one_way_only = 1;
		}
		else if(strcmp(argv[i],"-h")==0){
			printf("Usage: lab3 [- s/c] [-m] -p port -f file"
				   "Options:\n"
				   "   -s                  If set, the program runs as server.\n"
				   "   -c                  If set, the program runs as client.\n"
				   "[Server/Client parameters]\n"
				   "   -p <int>            The port for listening and connecting.\n"
				   "   -a IP ADDR          IP address for binding. [Default: Server binds in all available interfaces]\n"
				   "[Client parameters]\n"
				   "   -l <int>            UDP packet size.\n"
				   "   -b <int>            The bandwidth in bits/second of the data stream that the client should sent.\n"
				   "   -n <int>            Number of parallel data streams that the client should create.\n"
				   "   -t <int>            Duration in seconds. [Default: while(1)]\n"
				   "   -d                  Measure 1-way delay only\n");
			exit(EXIT_FAILURE);
		}
		else{

		}
	}
	printf("\nIs client %d, is server %d, port %d, address: %s, packet size %d, bandwidth %d, duration %d \n",is_client,is_server,port,ipstr,packet_size,bandwidth,duration);


	/*
	 * CHECK length packet size bandwidth
	 */
	 if(is_client==1 & (packet_size ==0 | bandwidth ==0)){
		 //Problemo
		 printf("\nFatal Error: Some parameters are wrong. Try again\n");
		free(ipstr);
		 exit(EXIT_FAILURE);
	 }
	 
	 
	 //			SOME CHECKS TO BE MADE			//
	 /*
	 *	CHECK one way flag and parallel
	 */

	 
	/*
	 * Depending the use arguments execute the appropriate functions
	 */
	if(is_server)
		exit_code = iperf_server(ipstr, port);
	if(is_client)
		exit_code = iperf_client(ipstr, port, packet_size, bandwidth, parallelstreams, duration, one_way_only);
	

	free(ipstr);
	return exit_code;
}

