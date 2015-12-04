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
#define MAX_CLIENTS 10

typedef struct{
	struct sockaddr_in6 client_addr;
	socklen_t client_addr_len;
	int fd;
}Client;

int
main(int argc, char** argv)
{
	int fd, optval, clientCount = 0, i, len;
	Client clients[MAX_CLIENTS];
	struct sockaddr_in6 server_addr;
	struct fd_set* read_set;
	char buf[BUFFER_SIZE];
	
	read_set = (struct fd_set *)malloc(sizeof(struct fd_set));

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
	Listen(fd, MAX_CLIENTS);
	
	FD_ZERO(read_set);
	
	// Init Clients
	for(i=0;i<MAX_CLIENTS;i++){clients[i].fd = -1;}
	
	for (;;) {	
		FD_SET(fd, read_set);	
		for(i=0;i<MAX_CLIENTS;i++){
			if(clients[i].fd == -1)
				continue;
			FD_SET(clients[i].fd, read_set);	
		}
		
		Select(fd+MAX_CLIENTS+1, read_set, (fd_set *) NULL,
					(fd_set *) NULL,
					(struct timeval *) NULL);

		// Gibts eine Anfrage?
		if(FD_ISSET(fd, read_set)){
			printf("Anfrage angekommen\n");
			// Neue Clients akzeptieren?
			if(clientCount < MAX_CLIENTS){
				i = 0;
				// Freie Position suchen
				while(clients[i].fd != -1){i++;}
				// Reinigung
				memset((void *) &(clients[i].client_addr), 0, sizeof(clients[i].client_addr));
				memset((void *) &(clients[i].client_addr_len), 0, sizeof(clients[i].client_addr_len));
				// Neuer Client
				clients[i].fd = Accept(fd, (struct sockaddr *) &(clients[i].client_addr),
												&(clients[i].client_addr_len));
				// Anzahl der Client +1
				clientCount++;
				// Zur Menge hinzufuegen
				FD_SET(clients[i].fd, read_set);
			}
		}
		
		for(i=0;i<MAX_CLIENTS;i++){
			// Suche einen Client
			if(clients[i].fd == -1)
				continue;
			// Wurde der Client belaestigt?
			if(FD_ISSET(clients[i].fd, read_set)){
				len = Recv(clients[i].fd, (void *) buf, sizeof(buf), 0);
				Write(1, buf, len);
				if(len == 0){
					
					// fd aus der Menge entfernen, fd schliessen, als Client loeschen, anzahl der Clients 						verringern
					FD_CLR(clients[i].fd, read_set);
					Close(clients[i].fd);
					clients[i].fd = -1;
					clientCount--;
				}			
			}
		}
	}
	Close(fd);

	return(0);
}
