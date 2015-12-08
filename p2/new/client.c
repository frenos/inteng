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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG
#define BUFFER_SIZE  (1<<16)
#define OUT_STREAMS 2048

int print_notification(void *buf);

int main(int argc, char **argv)
{
	int i, fd, done = 0, ppid = 1234, sid = 1, stay_in_order = 1, port = 55555, read_len = 0, flags = 0, notification;
	unsigned int infotype;
	struct sockaddr_in server_addr;
     	char buffer[BUFFER_SIZE], *serveraddr = "127.0.0.1";
     	struct iovec iov;
     	struct sctp_status status;
     	struct sctp_initmsg init;
     	struct sctp_sndinfo sinfo, rinfo;
     	struct sctp_paddrinfo paddrinfo;
	struct sctp_rtoinfo rtoinfo;
	struct sctp_paddrparams heartbeat;
	socklen_t opt_len, infolen;
	struct sctp_event event;
	uint16_t event_types[] = {SCTP_ASSOC_CHANGE,
				SCTP_PEER_ADDR_CHANGE,
				SCTP_REMOTE_ERROR,
				SCTP_SEND_FAILED_EVENT,
				SCTP_SHUTDOWN_EVENT,
				SCTP_ADAPTATION_INDICATION,
				SCTP_PARTIAL_DELIVERY_EVENT,
				SCTP_AUTHENTICATION_EVENT,
				SCTP_SENDER_DRY_EVENT
				};
	struct fd_set* rset;
	struct timeval timeout;


	if(argc == 6){
		serveraddr = argv[1];
		port = atoi(argv[2]);
		ppid = atoi(argv[3]);
		sid = atoi(argv[4]);
		stay_in_order = atoi(argv[5]);
	}else{
		printf("%s <\"ADRESSE\"> <PORT> <PPID> <STREAMID> <ORDERED>\n",argv[0]);
		printf("### DEFAULT ###\n");
		printf("%s \"%s\" %d %d %d %d\n",argv[0],serveraddr,port,ppid,sid,stay_in_order);
	}

	if(!stay_in_order)
    		sinfo.snd_flags = SCTP_UNORDERED;     	

	memset(&event, 0, sizeof(event));
	memset(&sinfo, 0, sizeof(sinfo));
	memset(&heartbeat, 0, sizeof(struct sctp_paddrparams));
	memset(&rtoinfo, 0, sizeof(struct sctp_rtoinfo));
	memset(&init, 0, sizeof(init));
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&status, 0, sizeof(status));
	rset = (struct fd_set *)malloc(sizeof(struct fd_set));
	memset(&timeout, 0, sizeof(timeout));
	memset(&paddrinfo, 0, sizeof(paddrinfo));

	event.se_on = 1;

	sinfo.snd_ppid = htonl(ppid);

	heartbeat.spp_hbinterval = 100;

	rtoinfo.srto_max = 2000;
	rtoinfo.srto_min = 50;

	init.sinit_num_ostreams = OUT_STREAMS;
	init.sinit_max_init_timeo = 200;

#ifdef HAVE_SIN_LEN
	server_addr.sin_len = sizeof(struct sockaddr_in);
#endif
	server_addr.sin_family      = AF_INET;
	server_addr.sin_port        = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(serveraddr);

	opt_len = (socklen_t)sizeof(status);

	sinfo.snd_sid = sid;

	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0){
      	 	perror("socket");
		exit(-1);
	}
	
	for(i = 0; i < sizeof(event_types)/sizeof(uint16_t); i++){
		event.se_type = event_types[i];
		if(setsockopt(fd, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event)) < 0)
			perror("setsockopt EVENTS");
	}


     	if (setsockopt(fd, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &heartbeat, (socklen_t)sizeof(heartbeat)) < 0){
       		perror("setsockopt heartbeat");
	}

     	if (setsockopt(fd, IPPROTO_SCTP, SCTP_RTOINFO , &rtoinfo, sizeof(rtoinfo)) < 0){
       		perror("setsockopt rtoinfo");
	}
     	
     	if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &init, (socklen_t)sizeof(init)) < 0){
       		perror("setsockopt init");
	}

	if (connect(fd, (const struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0){
		perror("connect");
		exit(-1);
	}

     	if (getsockopt(fd, IPPROTO_SCTP, SCTP_STATUS, &status, &opt_len) < 0){
		perror("getsockopt status");
	}
	
	if(sinfo.snd_sid > status.sstat_outstrms){
		sinfo.snd_sid = sinfo.snd_sid % status.sstat_outstrms;
	}
	
	if(rset == NULL){
		perror("malloc");
	}

	FD_ZERO(rset);
	FD_SET(fd, rset);

	timeout.tv_sec = 1;

	while(!done){
		flags = 0;
		FD_SET(0, rset);

		if(select(fd +1, rset, (fd_set *) NULL,	(fd_set *) NULL, (struct timeval *) &timeout) == -1){
			perror("select");
		}
		
		if(FD_ISSET(0, rset)) {
			memset(buffer, 0, BUFFER_SIZE);	
			iov.iov_base = buffer;
			if((read_len = read(0, (void *) buffer, sizeof (buffer))) == -1){
				perror("read");
			}

    			iov.iov_len = read_len;

			if(read_len==1){
				done = 1;
			}else{
				if(sctp_sendv(fd, (const struct iovec *)&iov, 1, NULL, 0, &sinfo, sizeof(sinfo), SCTP_SENDV_SNDINFO, 0) < 0) {
					perror("sctp_sendv");
				}
			}
		}else if(FD_ISSET(fd, rset)){
			memset(&rinfo, 0, sizeof(rinfo));
			infolen = (socklen_t)sizeof(rinfo);
			infotype = 0;

			memset(buffer, 0, BUFFER_SIZE);	
			iov.iov_base = buffer;
			iov.iov_len = BUFFER_SIZE;

			if(sctp_recvv(fd, &iov, 1, NULL, 0, &rinfo, &infolen, &infotype, &flags) < 0){
				perror("sctp_recvv");
			}else{
				FD_SET(fd, rset);
			}
				
			notification = 0;
			if (flags & MSG_NOTIFICATION) {			
			 	if((notification = print_notification(iov.iov_base)) == 1){
					done = 1;
				}
		       	}else{
				printf("INCOME: %s",buffer);
			}
		}else {
			memset(&paddrinfo, 0, sizeof(paddrinfo));
			opt_len = sizeof(paddrinfo);
			memcpy((void *)&paddrinfo.spinfo_address, (void *)&server_addr, sizeof(server_addr)); 
			if (getsockopt(fd, IPPROTO_SCTP, SCTP_GET_PEER_ADDR_INFO, &paddrinfo, &opt_len) < 0){
				perror("getsockopt paddrinfo");
			}else{
	
				printf("RTO: %d | MTU: %d | SRTT: %d\n",
					paddrinfo.spinfo_rto,
					paddrinfo.spinfo_mtu,
					paddrinfo.spinfo_srtt);
				fflush(stdout);
			}
		}
	}

	if(close(fd) < 0){
		perror("close");
	}

	free(rset);

	return(0);
}


int print_notification(void *buf){
	union sctp_notification *snp;
	snp = buf;
	printf("NOTIFICATION: ");
	switch(snp->sn_header.sn_type){
		case SCTP_ASSOC_CHANGE:
			printf("FANCY ASSOC CHANGE\n");
			break;
		case SCTP_PEER_ADDR_CHANGE:
			printf("FANCY PERR ADDR CHANGE\n");			
			break;
		case SCTP_REMOTE_ERROR:
			printf("FANCY REMOTE ERROR\n");
			break;
		case SCTP_SEND_FAILED_EVENT:
			printf("FANCY SEND FAILED EVENT\n");
			break;
		case SCTP_SHUTDOWN_EVENT:
			printf("FANCY SHUTDOWN EVENT\n");
			return 0;
			break;
		case SCTP_ADAPTATION_INDICATION:
			printf("FANCY ADAPTATION INDICATION\n");
			break;
		case SCTP_PARTIAL_DELIVERY_EVENT:
			printf("FANCY PARTIAL DELIVERY EVENT\n");
			break;
		case SCTP_AUTHENTICATION_EVENT:
			printf("FANCY AUTHENTICATION EVENT\n");
			break;
		case SCTP_SENDER_DRY_EVENT:
			printf("FANCY SENDER DRY EVENT\n");
			return 2;			
			break;
	}

	return 0;
}
