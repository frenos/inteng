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
#include <pthread.h>

#define BUFFER_SIZE (1<<16)
#define MAX_CLIENTS 1

typedef struct{
	struct sockaddr_in client_addr; // 16
	socklen_t client_addr_len; // 4
	int fd; //4
}Client;

void *client_handle(void *arg);

int main(int argc, char** argv)
{
	int fd, port = 55555;
	Client *client;
	pthread_t tid;
	char *serveraddr = "127.0.0.1";
	struct sockaddr_in server_addr;

	if(argc == 3){
		serveraddr = argv[1];
		port = atoi(argv[2]);
	}else{
		printf("%s <\"ADRESSE\"> <PORT> \n",argv[0]);
		printf("### DEFAULT ###\n");
		printf("%s \"%s\" %d \n",argv[0],serveraddr,port);
	}

	if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0){
		perror("socket"); 
	}

	memset((void *) &server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
#ifdef HAVE_SIN_LEN
	server_addr.sin_len = sizeof(struct sockaddr_in);
#endif
	server_addr.sin_addr.s_addr = inet_addr(serveraddr);
	server_addr.sin_port = htons(port);
	
	
	if(bind(fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
		perror("bind");
	}
	
	if(listen(fd, 1)){
		perror("listen");
	}

	while(1){
		client = (Client *)malloc(sizeof(Client));
		if(client == NULL){
			perror("malloc");
			break;
		}

		memset((void *) &(client->client_addr), 0, sizeof(client->client_addr));
		client->client_addr_len = sizeof(client->client_addr);
	
		/** ACCEPT TOTALER QUATSCH RFC VERWIRRUNG PUR BEI DER LAENGE */
		if((client->fd = accept(fd, (struct sockaddr *) &(client->client_addr), &(client->client_addr_len))) == -1){
			perror("accept");
		}
		pthread_create(&tid,NULL,client_handle,(void *)client);		
	}


	if(close(fd) < 0){
		perror("close");
	}
	
	return(0);
}

void *client_handle(void *arg){
	int read_len = 0, flags = 0, rcvinfo_on = 1;
	unsigned int rinfotype;
     	char *buffer;
     	struct iovec iov;
     	struct sctp_sndinfo sinfo;
	struct sctp_rcvinfo rinfo;
	socklen_t rinfolen;
	Client *client;

	client = (Client *)arg;

	buffer = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	if(buffer == NULL){
		perror("malloc");
		exit(1);
	}
	memset((void *)buffer, 0, sizeof(buffer));

	if (setsockopt(client->fd, IPPROTO_SCTP, SCTP_RECVRCVINFO, &rcvinfo_on, sizeof(rcvinfo_on)) < 0) {
       		perror("setsockopt SCTP_RECVRCVINFO");
       		exit(1);
    	}
	
	memset(&sinfo, 0, sizeof(sinfo));

	printf("CLIENT - %s:%d - fd:%d\n",inet_ntoa(client->client_addr.sin_addr),ntohs(client->client_addr.sin_port), client->fd);
		fflush(stdout);

	do{
		memset(&rinfo, 0, sizeof(rinfo));
		rinfolen = (socklen_t)sizeof(rinfo);
		rinfotype = 0;

		memset(buffer, 0, BUFFER_SIZE);	
		iov.iov_base = buffer;
		iov.iov_len = BUFFER_SIZE;
		
		if((read_len = sctp_recvv(client->fd, &iov, 1, (struct sockaddr *)&(client->client_addr), 
					&(client->client_addr_len), &rinfo, &rinfolen, &rinfotype, &flags)) < 0) {
			perror("sctp_recvv");
		}

		switch (rinfotype) {
         		case SCTP_RECVV_RCVINFO:
				printf("RCVINFO \n");
				printf("rcv_sid: %u\n",rinfo.rcv_sid);
				printf("rcv_ppid: %u\n",ntohl(rinfo.rcv_ppid));
				printf("rcv_flags: %u\n",rinfo.rcv_flags);
				sinfo.snd_sid = rinfo.rcv_sid;
				sinfo.snd_ppid = rinfo.rcv_ppid;
				sinfo.snd_flags = rinfo.rcv_flags;
			break;
		}

		printf("MESSAGE : %s",buffer);
		printf("-------------------\n");


		// ABSTURZ
		if(sctp_sendv(client->fd, (const struct iovec *)&iov, 1, (struct sockaddr *)&(client->client_addr), 
			0, &sinfo, sizeof(sinfo), SCTP_SENDV_SNDINFO, 0) < 0) {
			perror("sctp_sendv");
		}	

	}while(read_len>0 && buffer[0] != 0);

	if(close(client->fd) < 0){
		perror("close");
	}

	free(buffer);
	free(client);

	return NULL;

}
