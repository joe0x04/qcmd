#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXBUF 			10*1024
#define DEFAULTPORT		"27910"

uint8_t main(int8_t argc, char **argv)
{
	// show usage if we don't have enough args
	if (argc < 3) {
		printf("%s - Out of Band/Connectionless CLI for Quake 2\n", argv[0]);
		printf("Usage: %s <server DNS/IPv4/IPv6> <port number> <command>\n", argv[0]);
		printf("Ex: %s 198.51.100.1:27910 status\n", argv[0]);
		printf("Ex: %s q2.example.net:27911 rcon rc0np4ssw0rd map q2rdm2\n", argv[0]);
		printf("Ex: %s [2001:db8::dead:beef:babe]:1998 rcon rc0np4ssw0rd kick llamaplayer\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *hostname = argv[1];
	char *port;

	// not an IPv6 literal address
	if (hostname[0] != '[') {
		port = strstr(hostname, ":");
		if (!port) {
			port = DEFAULTPORT;
		} else {
			hostname[strlen(hostname) - strlen(port)] = '\0';
			*port++;	// get rid of the colon
		}
	}

	struct addrinfo hints, *res = 0;
	char buf[MAXBUF];
	uint8_t i;

	struct timeval tv;      // for socket timeout
    tv.tv_sec = 1;          // 1 second
    tv.tv_usec = 0;         // no microseconds
	
	memset(&hints, 0, sizeof(hints));
	//hints.ai_family		= AF_UNSPEC;	// ipv6 then v4
	hints.ai_family		= AF_INET;	// ipv4 only
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= 0;
	hints.ai_flags		= AI_ADDRCONFIG;

	
	// look up the hostname
	uint32_t err = getaddrinfo(hostname, port, &hints, &res);
	if (err != 0) {
		printf("failed to resolve remote socket address (err=%d)", err);
	}

	// creat the socket
	uint32_t fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (fd == -1) {
		printf("%s",strerror(errno));
	}

	// set a timeout in case server doesn't answer
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		perror("Error setting timeout");
	}

	// build the command
	strcpy(buf, "\xff\xff\xff\xff");	// sequence
	for (i=2; i<argc; i++) {
		strcat(buf, argv[i]);
		strcat(buf, "\x20");			// space
	}
	strcat(buf,"\x00");					// end with a null

	// send the datagram
	if (sendto(fd, buf, strlen(buf), 0, res->ai_addr, res->ai_addrlen) == -1) {
		printf("%s", strerror(errno));
	}

	freeaddrinfo(res);

	memset(&buf, 0x0, sizeof(buf));

	// get the response (block for timeout) if there is one
	size_t n_read = recvfrom(fd, buf, MAXBUF, 0, NULL, NULL);
	if (n_read < 0) {
		return EXIT_SUCCESS;
	}
	
	// do some house keeping on the returned data
	char output[MAXBUF];
	strncpy(output, buf+10, MAXBUF);       // skip the sequence and "print\n" at start
	printf("%s",output);

	return EXIT_SUCCESS;
}
