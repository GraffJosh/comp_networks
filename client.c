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
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define true 1

	int status, sfd, request_len;
	char RTT;
	ssize_t nread;
	char* URL;
	char* portnum;
	char* received_buffer;
	char* directory;
	char* hostname;
	char head_request[1000];
	int received_buffer_size;
	struct addrinfo *addr_info, *rp, *hints;

int main(int argc, char *argv[])
{
	received_buffer_size = 2000;
	received_buffer = realloc(received_buffer,sizeof(char)*received_buffer_size);
	URL = realloc(URL,(sizeof(char)*100));
	directory = realloc(directory,(sizeof(char)*100));
	hostname = realloc(hostname,(sizeof(char)*100));
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(hints));
	portnum = realloc(portnum,sizeof(long));
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
		return 1;	
	}else if(argc == 2){
		URL = argv[argc-1];	//the second to last arg is the 
		portnum = "80";
	}if(argc >= 3){
		URL = argv[argc-2];	//the second to last arg is the 
		portnum = argv[argc-1];		//last arg is port
		for (int i = 1; i < argc-2; ++i)
		{
			if(index(argv[i],'p') != NULL)
				RTT = true;			//user requested RTT 
		}
	}

	//DNS lookup
	if(getaddrinfo(URL,portnum,hints,&addr_info))
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
	//open local socket
	sfd = socket(addr_info->ai_family, addr_info->ai_socktype,addr_info->ai_protocol);
	//connect to remote host
	if (connect(sfd, addr_info->ai_addr, addr_info->ai_addrlen) != -1) //we're connected
		printf("success\n");


	if(addr_info == NULL)
	{
		printf("failed to connect\n");
		return 1;
	}
	freeaddrinfo(addr_info);

	char* _com_position;
	_com_position = strstr(URL, ".com") + 4;
	memcpy(directory, _com_position, strlen(URL)-(_com_position-URL));		//retreive the directory
	memcpy(hostname, URL, _com_position-URL);
	sprintf(head_request, "HEAD %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: Mozilla/5.0","/Obama.html",hostname);
	request_len = 80;//strlen(head_request)+2;
	printf("%s\n%d\n\n", head_request,request_len);
	//head_request data
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
	close(sfd);
	printf("Received %ld bytes: %s\n", (long) nread, received_buffer);

}