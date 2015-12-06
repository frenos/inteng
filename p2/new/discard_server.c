/*-
 * Copyright (c) 2013 Michael Tuexen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE (1<<16)
#define MAX_CLIENTS 1
#define TIMEOUT 0 // AUTOCLOSE OFF

int main(int argc, char** argv)
{
	int fd, port = 55555, read_len = 0, flags = 0, timeout;
	unsigned int infotype;
	struct sockaddr_in addr;
     	char buffer[BUFFER_SIZE], *serveraddr = "127.0.0.1";
     	struct iovec iov;
     	struct sctp_sndinfo info;
	socklen_t fromlen, infolen;
	struct fd_set* rset;

	if(argc == 3){
		serveraddr = argv[1];
		port = atoi(argv[2]);
	}else{
		printf("%s <\"ADRESSE\"> <PORT> \n",argv[0]);
		printf("### DEFAULT ###\n");
		printf("%s \"%s\" %d \n",argv[0],serveraddr,port);
	}

	if((fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0){
		perror("socket"); 
	}

	/** Configure auto-close timer. */
	timeout = TIMEOUT;
	if (setsockopt(fd, IPPROTO_SCTP, SCTP_AUTOCLOSE, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt SCTP_AUTOCLOSE");
	}
	/***/


	memset((void *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
#ifdef HAVE_SIN_LEN
	addr.sin_len = sizeof(struct sockaddr_in);
#endif
	addr.sin_addr.s_addr = inet_addr(serveraddr);
	addr.sin_port = htons(port);
	
	
	if(bind(fd, (const struct sockaddr *) &addr, sizeof(addr)) < 0){
		perror("bind");
	}
	
	if(listen(fd, MAX_CLIENTS)){
		perror("listen");
	}

	/** CREATE FD_SET */
	rset = (struct fd_set *)malloc(sizeof(struct fd_set));
	if(rset == NULL){
		perror("malloc");
	}
	FD_ZERO(rset);
	/***/
	
	fromlen = (socklen_t)sizeof(addr);

	while(1){

		FD_SET(fd, rset);

		if(FD_ISSET(fd, rset)){
			memset(&info, 0, sizeof(info));
			infolen = (socklen_t)sizeof(info);
			infotype = 0;

			memset(buffer, 0, BUFFER_SIZE);	
			iov.iov_base = buffer;
			iov.iov_len = BUFFER_SIZE;

			memset(&addr, 0, sizeof(addr));
			if((read_len = sctp_recvv(fd, &iov, 1, (struct sockaddr *)&addr, &fromlen, &info, &infolen, &infotype, &flags)) < 0) {
				perror("sctp_recvv");
			}

			printf("GOT A MESSAGE DUDE!\n");
			printf("###################\n");
			printf("FROM    : %s:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
			printf("LEN     : %d\n",read_len);
			printf("MESSAGE : %s",buffer);
			printf("BUT NOONE CARES!\n");
			printf("-------------------\n");
		}
	}


	if(close(fd) < 0){
		perror("close");
	}
	
	free(rset);

	return(0);
}
