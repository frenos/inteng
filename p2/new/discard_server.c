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

int main(int argc, char** argv)
{
	int fd, len, flags;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_len;
	char buf[BUFFER_SIZE];

	if((fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0){
		perror("socket"); 
		exit(-1);
	}
	printf("SOCKET SUCCESFULL (%d)\n",fd);	
	printf("---------------\n");

	memset((void *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
#ifdef HAVE_SIN_LEN
	server_addr.sin_len = sizeof(struct sockaddr_in);
#endif
	if(argc == 3){
		printf("GOT IP AND PORT FROM ARGV\n");
		printf("---------------\n");
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = htons(atoi(argv[2]));
	}else{
		printf("DEFAULT IP AND PORT (127.0.0.1:55555)\n");
		printf("---------------\n");
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(55555);
	}
	
	
	if(bind(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		close(fd);
		exit(-1);
	}
	printf("BIND SUCCESFULL\n");
	printf("---------------\n");	

	printf("LISTEN FOR NEW CLIENTS\n");
	printf("---------------\n");
	if(listen(fd, MAX_CLIENTS)){
		perror("listen");
	}
	
	client_len = (socklen_t)sizeof(struct sockaddr_in);	

	printf("HERE WE GO\n");
	printf("---------------\n");
	while(1){
		memset((void *)&client_addr, 0, sizeof(struct sockaddr_in));
		len = sctp_recvmsg(fd, (void *)buf, BUFFER_SIZE, (struct sockaddr *)&client_addr, &client_len, NULL, &flags);

		printf("GOT A MESSAGE DUDE!\n");
		printf("###################\n");
		printf("FROM    : %s:%d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
		printf("LEN     : %d\n",len);
		printf("MESSAGE : %s\n",buf);

		printf("\n\n AAAAND NOW -> TO THE BIN");
	}


	if(close(fd) < 0){
		perror("close");
	}

	return(0);
}
