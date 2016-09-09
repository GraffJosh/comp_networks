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


	int status, sfd;
	char* hostname;
	char* portnum;
	struct addrinfo *connection_addr_info;
	struct addrinfo *hints;

int main(int argc, char *argv[])
{

	hostname = realloc(hostname,(sizeof(char)*100));
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(hints));
	portnum = "80";
	
	//Beej.us textbook
	hints->ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints->ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints->ai_flags = AI_PASSIVE; // fill in my IP for me

	if (argc < 2)
	{
		printf("Not enough arguments\n");	
		printf("usage: ./client \'hostname\'");
		return 1;	
	}else{
		hostname = argv[1];
	}

	if(getaddrinfo(hostname,portnum,hints,&connection_addr_info))
	{
		printf("error retrieving info\n");
		return 1;
	}

	//man pages
	for (connection_addr_info; connection_addr_info != NULL; connection_addr_info = connection_addr_info->ai_next) 
	{
		sfd = socket(connection_addr_info->ai_family, connection_addr_info->ai_socktype,
			   connection_addr_info->ai_protocol);
		if (sfd == -1)
			continue;

		if (bind(sfd, connection_addr_info->ai_addr, connection_addr_info->ai_addrlen) == 0)
		{
			printf("success\n");
		   	break;                  /* Success */
		}

		close(sfd);
   	}



	freeaddrinfo(connection_addr_info);
}