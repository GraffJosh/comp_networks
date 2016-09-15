/*
@Author Josh Graff

Programming examples found from http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf as directed by Kinnicky
*/
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>       
#include <sys/stat.h>
#include <fcntl.h>

#define true 1

	int status, sfd, request_len,recieved_file_descriptor;
	char RTT_flag,head_flag;
	double RTT;
	ssize_t nread,nwrite;
	char* URL;
	char* portnum;
	char* received_buffer;
	char* directory;
	char* hostname;
	char get_request[1000];
	char head_request[1000];
	int received_buffer_size;
	struct addrinfo *addr_info, *rp, *hints;
	struct timeval start_time, end_time;
	unsigned long end_time_micro, start_time_micro;

void intHandler(int signal) {
	free(received_buffer);
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    act.sa_handler = intHandler;
    sigaction(SIGINT, &act, NULL);


	int i = 0;
	received_buffer_size = 5000000;
	received_buffer = realloc(received_buffer,sizeof(char)*received_buffer_size);
	URL = realloc(URL,(sizeof(char)*400));
	directory = realloc(directory,(sizeof(char)*200));
	hostname = realloc(hostname,(sizeof(char)*200));
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(hints));
	char ipstr[INET6_ADDRSTRLEN];
	//Man Pages
	hints->ai_family = AF_INET;    		//IPV4
	hints->ai_socktype = SOCK_STREAM; 	//TCP
	hints->ai_flags = 0;
	hints->ai_protocol = 0;


	//parse arguments
	if (argc < 2)
	{
		printf("Not enough arguments\n");	
		printf("usage: ./client \'flags\' \'URL\' \'port\'\n");
		printf("acceptable flags\n\'-h\'print this help message\n");
		printf("\'-p\'print RTT and data received as text\n");
		printf("\'-c\'send only a head request to server\n");
		return 1;	
	}else if(argc == 2){
		URL = argv[argc-1];	//the second to last arg is the 
		portnum = "80";
	}if(argc >= 3){
		URL = argv[argc-2];	//the second to last arg is the 
		portnum = argv[argc-1];		//last arg is port
		for (i = 1; i < argc-2; ++i)
		{
			if(index(argv[i],'p') != NULL)
				RTT_flag = true;			//user requested RTT 
			if(index(argv[i],'c') != NULL)
				head_flag = true;
			if(index(argv[i],'h') != NULL)
			{
				printf("usage: ./client \'flags\' \'URL\' \'port\'\n");
				printf("acceptable flags\n\'-h\'print this help message\n");
				printf("\'-p\'print RTT and data received as text\n");
				printf("\'-c\'send only a head request to server\n");
			}
		}
	}


	char* _com_position;
	if(strstr(URL, "https://"))
	{
		printf("Sorry, does not support secured connections.\n");
		exit(EXIT_FAILURE);
	}else if(strstr(URL, "http://"))
		URL = strstr(URL, "http://")+7;
		
	if(strstr(URL, ".com"))
		_com_position = strstr(URL, ".com") + 4;
	else if(strstr(URL, ".edu"))
		_com_position = strstr(URL, ".edu") + 4;
	else if(strstr(URL, ".org"))
		_com_position = strstr(URL, ".org") + 4;
	else if(strstr(URL, ".net"))
		_com_position = strstr(URL, ".net") + 4;
	else if(strstr(URL, ".us"))
		_com_position = strstr(URL, ".us") + 3;
	else if(strstr(URL, "localhost"))
		_com_position = strstr(URL, "localhost")+9;
	else{
		printf("Unrecognized TLD.\n");
		exit(EXIT_FAILURE);
	}
	//printf("%d\n",strlen(URL) );
	//if there is a TLD attached
	memcpy(directory, _com_position, strlen(URL)-(_com_position-URL));		//parse the directory
	memcpy(hostname, URL, _com_position-URL);								//parse the hostname
	if(directory[0] == '/')
		memmove(directory, directory+1,strlen(directory));
	//construct the head request
	sprintf(head_request, "HEAD /%s HTTP/1.0\r\nHost: %s\r\nConnection: Keep-Alive\r\nUser-Agent: Mobile/7B405\r\n\r\n",directory, hostname);
	sprintf(get_request, "GET /%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\nUser-Agent: Mobile/7B405\r\n\r\n",directory, hostname);
	

	//DNS lookup
	if(getaddrinfo(hostname,portnum,hints,&addr_info))
	{
		printf("error retrieving info\n");
		return 1;
	}

/*
//prints the IPAddr; from the text that kinnicky linked us.
	for(rp = addr_info;rp != NULL; rp = rp->ai_next) {
		void *addr;
		char *ipver;
		// get the rpointer to the address itself,
		// different fields in Irpv4 and Irpv6:
		if (rp->ai_family == AF_INET) { // Irpv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)rp->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "Ipv4";
		} else { // Irpv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)rp->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "Ipv6";
		}
		// convert the Irp to a string and print it:
		inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n", ipver, ipstr);
	}
*/

//Get a head request to resize the buffer for the page.

	//open local socket according to the parameters specified previously
	sfd = socket(addr_info->ai_family, addr_info->ai_socktype,addr_info->ai_protocol);
	//connect to remote host (and time it)
	gettimeofday(&start_time,NULL);
	if (connect(sfd, addr_info->ai_addr, addr_info->ai_addrlen) != -1) //we're connected
	gettimeofday(&end_time,NULL);

	if(addr_info == NULL)
	{
		printf("failed to connect\n");
		return 1;
	}else{
		freeaddrinfo(addr_info);
	}



	/*//get a head_request to resize the buffer
	request_len = strlen(head_request);
	if((status = send(sfd, head_request, request_len,0)) != request_len) 
	{
		fprintf(stderr, "partial/failed write\n");
		exit(EXIT_FAILURE);
	}
	//receive data
	nread = recv(sfd, received_buffer, received_buffer_size,0);
	if (nread == -1) 
	{
		perror("read");
		exit(EXIT_FAILURE);
	}
	//printf("Received %ld bytes:\n%s\n", (long) nread,received_buffer); //print return of head request
	char* _size_position;
	char* _size_end;
	char _size_char[30];
	if(strstr(received_buffer, "Content-Length: "))
	{
		_size_position = strstr(received_buffer, "Content-Length: ") + 16;
		_size_end = strstr(_size_position, "\r\n");
		
		memcpy(_size_char, _size_position, _size_end-_size_position);
		received_buffer_size = sizeof(char)*(atoi(_size_position)+1000);// 245230*sizeof(char);//
	}else{
		received_buffer_size = sizeof(char)*200000;
	}
		received_buffer = realloc(received_buffer,received_buffer_size);
	*/


	//get_request data get the actual data
	if(!head_flag)
	{
		request_len = strlen(get_request);
		if((status = send(sfd, get_request, request_len,0)) != request_len) 
		{
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
	}else{
		request_len = strlen(head_request);
		if((status = send(sfd, head_request, request_len,0)) != request_len) 
		{
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
	}

	//receive data
	nread = recv(sfd, received_buffer, received_buffer_size,MSG_WAITALL);
	if (nread == -1) 
	{
		perror("read");
		exit(EXIT_FAILURE);
	}
	//close socket
	close(sfd);


	received_buffer[received_buffer_size] = '\0';
	if((recieved_file_descriptor = open(directory,O_RDONLY))==-1)
	{					
		perror(error_text);
	}else{
		nwrite = write(recieved_file_descriptor,received_buffer, received_buffer_size);
		if(nwrite==-1)
		{	
			perror(error_text);
			exit_handler(1);
		}
	}
	//print timing if requested
	start_time_micro = start_time.tv_usec + (1000000 * start_time.tv_sec);
	end_time_micro =  end_time.tv_usec+(1000000 * end_time.tv_sec);
	RTT = end_time_micro-start_time_micro;
	if(RTT_flag)
		printf("\n\n%s\n\nReceived %ld bytes in %f miliseconds.\n",received_buffer,(long)nread,RTT/1000);


	free(received_buffer);
	free(directory);
	free(hostname);
}



