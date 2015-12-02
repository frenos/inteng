#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "Socket.h"

int Socket(int family, int type, int protocol){
	int n;
	if((n=socket(family,type,protocol))==-1){
		perror("socket");
		exit(1);
	}
	return n;
}
int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen){
	int n;
	if((n=bind(fd,addr,addrlen))==-1){
		perror("bind");
		exit(1);
	}
	return n;
}
ssize_t Recvfrom(int fd, void *buf, size_t buflen,int flags, struct sockaddr *from,socklen_t *addrlen){
	ssize_t n;
	if((n=recvfrom(fd,buf,buflen,flags,from,addrlen))==-1){
		perror("recvfrom");
		exit(1);
	}
	return n;
}
ssize_t Sendto(int fd, void *buf, size_t buflen,int flags, const struct sockaddr *to,socklen_t addrlen){
	ssize_t n;
	if((n=sendto(fd,buf,buflen,flags,to,addrlen))==-1){
		perror("sendto");
		exit(1);
	}
	return n;
}
int Close(int fd){
	int n;
	if((n=close(fd))==-1){
		perror("close");
		exit(1);
	}
	return n;
}

int Connect(int fd,const struct sockaddr *addr,socklen_t len){
	int n;
	if((n=connect(fd,addr,len))==-1){
		perror("connect");
		exit(1);
	}
	return n;
}

ssize_t Recv(int fd, void *buf, size_t buflen, int flags){
	ssize_t n;
	if((n=recv(fd,buf,buflen,flags))==-1){
		perror("recv");
		exit(1);
	}
	return n;
}

ssize_t Send (int fd, void *buf, size_t buflen, int flags){
	ssize_t n;
	if((n=send(fd,buf,buflen,flags))==-1){
		perror("send");
		exit(1);
	}
	return n;
}

ssize_t Read (int fd, void *buf, size_t buflen){
	ssize_t n;
	if((n=read(fd,buf,buflen))==-1){
		perror("read");
		exit(1);
	}
	return n;
}

ssize_t Write (int fd, void *buf, size_t buflen){
	ssize_t n;
	if((n=write(fd,buf,buflen))==-1){
		perror("write");
		exit(1);
	}
	return n;
}

int Listen(int fd, int backlog){
	int n;
	if((n=listen(fd,backlog))==-1){
		perror("listen");
		exit(1);
	}
	return n;
}

int Accept(int fd, struct sockaddr* addr, socklen_t *len){
	int n;
	if((n=accept(fd,addr,len))==-1){
		perror("accept");
		exit(1);
	}
	return n;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout){
	int n;
	if((n=select(nfds,readfds,writefds,exceptfds,timeout))==-1){
		perror("select");
		exit(1);
	}
	return n;
}

int Shutdown(int fd, int how){
	int n;
	if((n=shutdown(fd, how))==-1){
		perror("shutdown");
		exit(1);
	}
	return n;
}


