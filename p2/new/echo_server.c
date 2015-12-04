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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "Socket.h"

#define BUFFER_SIZE (1<<16)

typedef struct{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
	int fd;
}Client;

void *client_handle(void *arg);


int
main(int argc, char** argv)
{
	int fd, optval;
	Client *client;
	struct sockaddr_in6 server_addr;
	pthread_t tid;

	fd = Socket(AF_INET6, SOCK_STREAM, 0);
	optval = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
	optval = 0;
	setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof optval);
	memset((void *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin6_family = AF_INET6;
#ifdef HAVE_SIN_LEN
	server_addr.sin6_len = sizeof(struct sockaddr_in6);
#endif
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(atoi(argv[1]));
	Bind(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
	Listen(fd, 1);
	for (;;) {
		client = (Client *)malloc(sizeof(Client));		
		memset((void *) &(client->client_addr), 0, sizeof(client->client_addr));
		memset((void *) &(client->client_addr_len), 0, sizeof(client->client_addr_len));
		client->fd = Accept(fd, (struct sockaddr *) &(client->client_addr), &(client->client_addr_len));
		pthread_create(&tid,NULL,client_handle,(void *)client);
	}
	Close(fd);

	return(0);
}

void *client_handle(void *arg){
	char *buf;
	Client *client;
	size_t len;

	buf = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	memset((void *) buf, '\0', sizeof(buf));
	client = (Client *)arg;

	do{
		len = Recv(client->fd, (void *) buf, sizeof(buf), 0);
		Send(client->fd, (void *) buf, len, 0);
	}while(len > 0);
	Close(client->fd);
	free(client);
	free(buf);
	
	return NULL;
}
