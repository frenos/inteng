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
#include "Socket.h"

#define BUFFER_SIZE (1<<16)

int
main(int argc, char** argv)
{
	int fd, optval, cfd;
	struct sockaddr_in6 server_addr, client_addr;
	socklen_t addr_len, client_addr_len;
	time_t ticks;
	char buf[BUFFER_SIZE];

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
		memset((void *) buf, '\0', sizeof(buf));
		addr_len = (socklen_t) sizeof(client_addr);
		memset((void *) &client_addr, 0, sizeof(client_addr));
		client_addr_len = (socklen_t) sizeof(client_addr);
		cfd = Accept(fd, (struct sockaddr *) &client_addr, &client_addr_len);
		ticks = time(NULL);
		snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
		printf("%ld\r\n", strlen(buf));
		Sendto(cfd, (void *) buf, strlen(buf), 0, (struct sockaddr *)&client_addr, client_addr_len);
		Close(cfd);
	}
	Close(fd);

	return(0);
}
