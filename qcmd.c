#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXBUF 10*1024

int main(int argc, char **argv)
{
	// show usage if we don't have enough args
	if (argc < 4)
	{
		printf("%s - Out of Band/Connectionless CLI for Quake 2\n", argv[0]);
		printf("Usage: %s <server DNS/IPv4/IPv6> <port number> <command>\n", argv[0]);
		printf("Ex: %s 198.51.100.1 27910 status\n", argv[0]);
		printf("Ex: %s q2.example.net 27911 rcon rc0np4ssw0rd map q2rdm2\n", argv[0]);
		printf("Ex: %s 2001:db8::dead:beef:babe 1998 rcon rc0np4ssw0rd kick llamaplayer\n", argv[0]);
		exit(1);
	}

	const char* hostname = argv[1];
	const char* portname = argv[2];
	struct addrinfo hints, *res=0;
	char buf[MAXBUF];
	int i;

	struct timeval tv;      // for socket timeout
    tv.tv_sec = 1;          // 1 second
    tv.tv_usec = 0;         // no microseconds
	
	memset(&hints, 0, sizeof(hints));
	hints.ai_family		= AF_UNSPEC;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= AI_ADDRCONFIG;
	
	// look up the hostname
	int err = getaddrinfo(hostname, portname, &hints, &res);
	if (err != 0) 
	{
		printf("failed to resolve remote socket address (err=%d)", err);
	}

	// creat the socket
	int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd == -1) 
	{
		printf("%s",strerror(errno));
	}

	// set a timeout in case server doesn't answer
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		perror("Error setting timeout");
	}

	// build the command
	strcpy(buf, "\xff\xff\xff\xff");        // sequence
	for (i=3; i<argc; i++)
	{
			strcat(buf, argv[i]);
			strcat(buf, "\x20");    // space
	}
	strcat(buf,"\x00");             // end with a null

	// send the datagram
	if (sendto(fd, buf, sizeof(buf), 0, res->ai_addr, res->ai_addrlen) == -1) 
	{
		printf("%s", strerror(errno));
	}

    int buf_len;
	int n_read = recvfrom(fd, buf, MAXBUF, 0, NULL, NULL);
	if (n_read < 0)
	{
			perror("Problem in recvfrom");
			exit(1);
	}

	char output[10*1024];
	strncpy(output, buf+10, 10*1024);       // skip the sequence and "print\n" at start
	printf("%s",output);


	return 0;
}



