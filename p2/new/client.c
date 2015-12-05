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

#define BUFFER_SIZE  (1<<16)
#define OUT_STREAMS 2048

char* sgets(char *buffer, size_t size);

int
main(int argc, char **argv)
{
	int fd, done = 0, ppid = 1234, sid = 1, stay_in_order = 0, port = 55555;
	struct sockaddr_in client_addr;
     	char buffer[BUFFER_SIZE], *serveraddr = "127.0.0.1";
     	struct iovec iov;
     	struct sctp_status status;
     	struct sctp_initmsg init;
     	struct sctp_sndinfo info;
     	//struct sctp_setadaptation ind;
	socklen_t opt_len;

	if(argc == 6){
		serveraddr = argv[1];
		port = atoi(argv[2]);
		ppid = atoi(argv[3]);
		sid = atoi(argv[4]);
		stay_in_order = atoi(argv[5]);
	}else{
		printf("%s <\"ADRESSE\"> <PORT> <PPID> <STREAMID> <ORDERED>\n",argv[0]);
		exit(1);
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0) {
      	 	perror("socket");
       		exit(1);
     	}	

	/** TELL THE OTHER DUDE HOW MUCH STREAMS U WANT */
	memset(&init, 0, sizeof(init));
     	init.sinit_num_ostreams = OUT_STREAMS;
     	if (setsockopt(fd, IPPROTO_SCTP, SCTP_INITMSG, &init, (socklen_t)sizeof(init)) < 0) {
       		perror("setsockopt OUT_STREAMS");
       		exit(1);
     	}
	/***/

	/** SET THE PPID AND (UN)ORDERED */
	memset(&info, 0, sizeof(info));
	info.snd_ppid = htonl(ppid);
	if(!stay_in_order){
    		info.snd_flags = SCTP_UNORDERED;
	}
	/***/

	/** SET THE DATAVECTOR */
	memset(buffer, 0, BUFFER_SIZE);
	iov.iov_base = buffer;
    	iov.iov_len = BUFFER_SIZE;
	/***/

	/** LETS GET READY TO RUMBLE */
	memset(&client_addr, 0, sizeof(client_addr));
#ifdef HAVE_SIN_LEN
	client_addr.sin_len = sizeof(struct sockaddr_in);
#endif
	client_addr.sin_family      = AF_INET;
	client_addr.sin_port        = htons(port);
	client_addr.sin_addr.s_addr = inet_addr(serveraddr);

	if (connect(fd, (const struct sockaddr *)&client_addr, sizeof(struct sockaddr_in)) < 0) {
		perror("connect");
		exit(1);
     	}


	/** SET THE CORRECT SID (HINT: GET NUMBER OF OUTGOING STREAMS AND CHECK) 
	    MUSST BE DONE AFTER THE CONNECT */
	memset(&status, 0, sizeof(status));
     	opt_len = (socklen_t)sizeof(status);
     	if (getsockopt(fd, IPPROTO_SCTP, SCTP_STATUS, &status, &opt_len) < 0) {
		perror("getsockopt SET_CORRECT_STREAMS");
		exit(1);
	}

	if(sid > status.sstat_outstrms){
		info.snd_sid = sid % status.sstat_outstrms;
		printf("ONLY %d STREAMS AVAILABLE - OUR SID %d | NEW SID %d", status.sstat_outstrms, sid, info.snd_sid);
	}
	/***/

	while(!done){
		if(sgets(buffer,BUFFER_SIZE)==NULL){
			done = 1;
		}else{
			if(sctp_sendv(fd, (const struct iovec *)&iov, 1, NULL, 0, &info, sizeof(info), SCTP_SENDV_SNDINFO, 0) < 0) {
				perror("sctp_sendv");
				exit(1);
			}
		}
	}

	if(close(fd)<0){
		perror("close");
	}
	return(0);
}

char* sgets(char *buffer, size_t size)
{
   size_t i;
   for ( i = 0; i < size - 1; ++i )
   {
      int ch = fgetc(stdin);
      if ( ch == '\n' || ch == EOF )
      {
         break;
      }
      buffer[i] = ch;
   }

   if(i == 0){
      return NULL;
   }

   buffer[i] = '\0';
   return buffer;
}
