/*
@Author Josh Graff

Programming examples found from http://beej.us/guide/bgnet/output/print/bgnet_USLetter.pdf as directed by Kinnicky
*/
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>       
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>



struct addrinfo *addr_info, *rp, *hints;
struct sockaddr_storage incoming_addr;
char *portnum, *error_text,*received_buffer,*send_buffer;
char *directory,*filename,*connection_type,*http_response;
char error_status[15];
int received_buffer_size,send_buffer_size,http_response_size;
int filename_size,connection_type_size,directory_size,sfd,incoming_fd,true,not_found;
ssize_t nread;
socklen_t sin_size;
int send_file_descriptor;

void exit_handler(int signal) {
	switch (signal)
	{
		case 0:
		//printf("Close Thread\n");
		exit(0);
		case 1://internal error
		free(send_buffer);
		free(received_buffer);
		free(connection_type);
		free(filename);
		printf("exit on error %d\n", signal);
		exit(1);
		case 2://control-c
		free(send_buffer);
		free(received_buffer);
		free(connection_type);
		free(filename);
		printf("Exit.\n");
		exit(0);
		case 3://thread close
		//printf("Close Thread\n");
		exit(0);
		default:
		//printf("Close Thread\n");
		exit(0);
	}
	printf("Exited.\n");
	exit(0);
}


int main(int argc, char *argv[])
{

    struct sigaction act;
    act.sa_handler = exit_handler;
    sigaction(SIGINT, &act, NULL);

	received_buffer_size = 20000;
	received_buffer = realloc(received_buffer,sizeof(char)*received_buffer_size);
	filename_size = 500;
	filename = realloc(filename,sizeof(char)*filename_size);
	connection_type_size = 500;
	connection_type = realloc(connection_type,sizeof(char)*connection_type_size);
	directory_size = 500;
	directory = realloc(directory,sizeof(char)*directory_size);
	send_buffer_size = 6000000;
	send_buffer = realloc(send_buffer,sizeof(char)*send_buffer_size);
	http_response_size = 250;
	http_response = realloc(http_response,sizeof(char)*http_response_size);
	portnum = realloc(portnum,(sizeof(char)*20));
	hints = realloc(hints,sizeof(*hints));
	error_text = realloc(error_text,sizeof(char)*80);
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
	if(getaddrinfo("cccwork3.wpi.edu",portnum,hints,&addr_info))
	{
		printf("error retrieving info\n");
		exit_handler(1);
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
		printf("%s: %s\n", ipver, ipstr);



		if ((sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol)) == -1) {
						
			perror(error_text);
			printf("%s\n", error_text);
			perror("server: socket");
			continue;
		}

		if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &true,sizeof(int)) == -1) 
		{			
			perror(error_text);
			printf("%s\n", error_text);
			perror("SocketInUseError");
			exit_handler(1);
		}

		if(bind(sfd, rp->ai_addr,rp->ai_addrlen)==-1)
		{
			perror(error_text);
			printf("%s\n", error_text);
			close(sfd);
			printf("BindError\n");
			continue;
		}
	}
	freeaddrinfo(addr_info);

/*	if(!rp)
	{
		perror(error_text);
		printf("%s\n", error_text);
		printf("failed to bind\n");
		exit_handler(1);
	}*/
	if(listen(sfd, 10)==-1)
	{			
		perror(error_text);
		printf("%s\n", error_text);
		printf("ListenError\n");
		exit_handler(1);
	}

	printf("Listening on port: %s\n", portnum);

	while(1)
	{
		socklen_t _addr_len = sizeof(incoming_addr);
		if((incoming_fd=accept(sfd,(struct sockaddr *)&incoming_addr,&_addr_len))==-1)
		{			
			perror(error_text);
			printf("%s\n", error_text);
			printf("AcceptError\n");
		}

		if (!fork()) { // this is the child process
			close(sfd); // child doesn't need the listener
			//receive data
			memset(received_buffer, 0, received_buffer_size);
			memset(send_buffer, 0, send_buffer_size);
			not_found = 0;
			nread = recv(incoming_fd, received_buffer, received_buffer_size,0);
		if (nread == -1) 
		{	perror("read");
			exit_handler(1);}

			char* _file_name_position,*_connection_type_position;
			char* _file_name_end,*_connection_type_end;
			char _size_char[30];
			if(strstr(received_buffer, "GET /"))
			{
				_file_name_position = strstr(received_buffer, "GET /") + 5;
				_file_name_end = strstr(_file_name_position, "HTTP");
				if(_file_name_end-_file_name_position > filename_size)
				{
					printf("filename too long\n");
					not_found = 404;
				}
				memcpy(filename, _file_name_position, (_file_name_end-_file_name_position)-1);
			}else if(strstr(received_buffer, "HEAD /"))
			{
				_file_name_position = strstr(received_buffer, "HEAD /") + 5;
				_file_name_end = strstr(_file_name_position, "HTTP");
				if(_file_name_end-_file_name_position > filename_size)
				{
					printf("filename too long\n");
					not_found = 404;
				}
				memcpy(filename, _file_name_position, (_file_name_end-_file_name_position)-1);
			}		
			if(strstr(received_buffer, "Connection: "))
			{
				_connection_type_position = strstr(received_buffer, "Connection: ") + 12;
				_connection_type_end = strstr(_connection_type_position, "\r\n");
				if(_connection_type_end-_connection_type_position > connection_type_size)
				{
					printf("Connection type error\n");
					exit_handler(1);
				}
				memcpy(connection_type, _connection_type_position, (_connection_type_end-_connection_type_position));
			}




			if(strlen(filename)<=1)
			{
				filename = "index.html";
			}

			printf("filename:%s\n", filename);

			if((send_file_descriptor = open(filename,O_RDONLY))==-1)
			{					
				perror(error_text);
				not_found = 404;
			}

			nread = read(send_file_descriptor,send_buffer, send_buffer_size-1);
			if(nread==-1)
			{	
				perror(error_text);
				not_found = 404;
			}	
			if(not_found)
			{
				sprintf(http_response,"HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: %s\r\n\r\n",(int)nread,connection_type);
				send(incoming_fd, http_response, strlen(http_response),0);
			}else{
				sprintf(http_response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: %s\r\n\r\n",(int)nread+1,connection_type);
				send(incoming_fd, http_response, strlen(http_response),0);
				if(strstr(received_buffer, "GET"))
					send(incoming_fd, send_buffer,nread+strlen(http_response),0);
			}
			



			close(incoming_fd);
			exit_handler(3);
		}
		close(incoming_fd);
		
	}
}