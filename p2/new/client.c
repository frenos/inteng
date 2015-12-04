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
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include "Socket.h"

#define BUFFER_SIZE  (1<<16)
#define MESSAGE_SIZE (9216)

int
main(int argc, char **argv)
{
	int fd;
	int n;
	struct fd_set* rset;
	int done=0;
	char buf[BUFFER_SIZE];
	struct addrinfo *host, *buffer, hints;
	int error;
	rset = (struct fd_set *)malloc(sizeof(struct fd_set));
	

	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_TCP;


	error = getaddrinfo(argv[1],argv[2],&hints,&host);
	if(error!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		exit(EXIT_FAILURE);
	}

	for(buffer = host; buffer != NULL; buffer = buffer->ai_next){
		fd = socket(buffer->ai_family, buffer->ai_socktype, buffer->ai_protocol);
		if(fd == -1){
			continue;
		}
		error = connect(fd, buffer->ai_addr, buffer->ai_addrlen);
		if(error == 0){ 
			break;
		}
		Close(fd);
	}
	if(buffer == NULL){
		printf("Kein Zielhost gefunden!\n");
		exit(EXIT_FAILURE);
	}
	printf("Connected\n");
	memset((void *) buf, '\0', sizeof(buf));
	FD_ZERO(rset);
	
	while(!done){
		FD_SET(0, rset);
		FD_SET(fd, rset);
		
		Select(fd +1, rset, (fd_set *) NULL,
					(fd_set *) NULL,
					(struct timeval *) NULL);
		if(FD_ISSET(0, rset)) {
			n = Read(0, (void *) buf, sizeof (buf));
			if(n==0)
				Shutdown(fd, SHUT_WR);
			else
				Send(fd, (void *) buf, (size_t) n, 0);
		}
		if(FD_ISSET(fd, rset)){
			n =Recv(fd, (void *) buf, sizeof(buf), 0);
			if(n==0)
				done = 1;
			else
				Write(1, buf, n);
		}
	}

	close(fd);
	free(rset);
	return(0);
}
