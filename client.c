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

	int status, sfd, request_len;
	ssize_t nread;
	char* hostname;
	char* portnum;
	char* received_buffer;
	int received_buffer_size;
	struct addrinfo *addr_info, *rp, *hints;

int main(int argc, char *argv[])
{
	request_len =85;
	received_buffer_size = 2000;
	received_buffer = realloc(received_buffer,sizeof(char)*received_buffer_size);
	hostname = realloc(hostname,(sizeof(char)*100));
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(hints));
	portnum = "80";
	char ipstr[INET6_ADDRSTRLEN];
	//Man Pages
	hints->ai_family = AF_INET;    		//IPV4
	hints->ai_socktype = SOCK_STREAM; 	//TCP
	hints->ai_flags = 0;
	hints->ai_protocol = 0;


	if (argc < 2)
	{
		printf("Not enough arguments\n");	
		printf("usage: ./client \'hostname\'\n");
		return 1;	
	}else{
		hostname = argv[1];
	}

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

	sfd = socket(addr_info->ai_family, addr_info->ai_socktype,addr_info->ai_protocol);

	if (connect(sfd, addr_info->ai_addr, addr_info->ai_addrlen) != -1) //we're connected
		printf("success\n");


	if(addr_info == NULL)
	{
		printf("failed to connect\n");
		return 1;
	}
	freeaddrinfo(addr_info);



	//request data
	if((status = send(sfd, "GET /Obama.html HTTP/1.0\r\nHost: Joshuagraff.com\r\nUser-Agent: Mobile/7B405\r\n\r\n", request_len,0)) != request_len) 
	{
		fprintf(stderr, "partial/failed write\n");
		exit(EXIT_FAILURE);
	}
	printf("%d\n", status);

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