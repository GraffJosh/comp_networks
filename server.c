/*
@Author Josh Graff

Programming examples found from http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf as directed by Kinnicky
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>


struct addrinfo *addr_info, *rp, *hints;
struct sockaddr_storage incoming_addr;
char* portnum;
int sfd,incoming_fd,true;
socklen_t sin_size;

int main(int argc, char *argv[])
{
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(hints));
		//Man Pages
	hints->ai_family = AF_INET;    		//IPV4
	hints->ai_socktype = SOCK_STREAM; 	//TCP
	hints->ai_flags = AI_PASSIVE;		
	true = 1;
	//parse arguments
	if (argc < 2)
	{
		printf("default port 80 used\n");
		portnum = "80";
	}else if(argc == 2){
		memcpy(portnum, argv[1],6);//portnum = argv[1];	//the second to last arg is the 
	}


	//DNS lookup
	if(getaddrinfo(NULL,portnum,hints,&addr_info))
	{
		printf("error retrieving info\n");
		exit(1);
	}
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
		char ipstr[100];
		inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
		printf(" %s: %s\n", ipver, ipstr);
	}
	//cycle through the linked list of address information until we find one that works
	for(rp = addr_info; rp != NULL; rp = rp->ai_next) {
		if ((sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &true,sizeof(int)) == -1) 
		{
			perror("SocketInUseError");
			exit(1);
		}

		if(bind(sfd, rp->ai_addr,rp->ai_addrlen)==-1)
		{
			close(sfd);
			printf("BindError\n");
			continue;
		}
	}
	freeaddrinfo(addr_info);

	if(!rp)
	{
		printf("failed to bind\n");
		exit(1);
	}
	if(listen(sfd, 10)==-1)
	{
		printf("ListenError\n");
		exit(1);
	}

	printf("Listening on port: %s\n", portnum);

	while(1)
	{
		socklen_t _addr_len = sizeof(incoming_addr);
		if((incoming_fd=accept(sfd,(struct sockaddr *)&incoming_addr,&_addr_len))==-1)
		{
			printf("AcceptError\n");
		}

		send(incoming_fd, "JPGIndustries",13,0);

		close(incoming_fd);
	}
}