/*
============================================================================
        Name            : qcmd.c
        Author          : Joe Reid (joe@joereid.com) - claire, Dr_Evil, scarred
        Version         : 0.1.1
        Date            : 2013-06-10
        Copyright       : Do want you want with this....
        Description     : Allows you to run Quake (1|2|3|4|2w) out-of-band (connectionless)
                        : commands
        Usage           : qcmd <ip/hostname> <port> <command>
        Examples        : qcmd 1.1.1.1 27910 status
                        : qcmd q2.domain.com 27911 rcon rc0np4ssw0rd map q2dm2
============================================================================
*/

#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>

#define MAXBUF 10*1024

int main( int argc, char **argv ) {
        int sk;
        struct sockaddr_in server;
        struct hostent *hp;
        char buf[MAXBUF];
        int buf_len;
        int n_sent;
        int n_read;
        struct timeval tv;      // for socket timeout
        tv.tv_sec = 1;          // 1 second
        tv.tv_usec = 0;         // no microseconds

        // show usage
        if (argc < 4)
        {
                printf("%s - Out of Band/Connectionless CLI for Quake 2\n", argv[0]);
                printf("Usage: %s <server name/IP> <port number> <command>\n", argv[0]);
                printf("Ex: %s 1.1.1.1 27910 status\n", argv[0]);
                printf("Ex: %s q2.server.com 27911 rcon rc0np4ssw0rd map q2rdm2\n", argv[0]);
                exit(0);
        }

        if ((sk = socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
        {
                printf("Problem creating socket\n");
                exit(1);
        }

        // set a timeout
        if (setsockopt(sk, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        {
                perror("Error");
        }

        server.sin_family = AF_INET;

        if ((hp = gethostbyname(argv[1])) == 0)
        {
                printf("Invalid or unknown host\n");
                exit(1);
        }

        memcpy(&server.sin_addr.s_addr, hp->h_addr, hp->h_length);

        server.sin_port = htons(atoi(argv[2]));

        // build the command
        int i;
        strcpy(buf, "\xff\xff\xff\xff");        // sequence
        for (i=3; i<argc; i++)
        {
                strcat(buf, argv[i]);
                strcat(buf, "\x20");    // space
        }
        strcat(buf,"\x00");             // end with a null

        buf_len = strlen(buf);

        n_sent = sendto(sk,buf, buf_len, 0, (struct sockaddr*) &server, sizeof(server));
        if (n_sent < 0)
        {
                perror("Problem sending data");
                exit(1);
        }

        n_read = recvfrom(sk,buf,MAXBUF,0,NULL,NULL);
        if (n_read < 0)
        {
                perror("Problem in recvfrom");
                exit(1);
        }

        char output[10*1024];
        strncpy(output, buf+10, 10*1024);       // skip the sequence and "print\n" at start
        printf("%s",output);

        return(0);
}
