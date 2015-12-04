#ifndef _SOCKET_H_
#define _SOCKET_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int Socket(int family, int type, int protocol);
int Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t Recvfrom(int fd, void *buf, size_t buflen,int flags, struct sockaddr *from,socklen_t *addrlen);
ssize_t Sendto(int fd, void *buf, size_t buflen, int flags, const struct sockaddr *to, socklen_t addrlen);
int Close(int fd);
int Connect(int fd, const struct sockaddr *addr, socklen_t len);
ssize_t Recv(int fd, void *buf, size_t buflen, int flags);
ssize_t Send (int fd, void *buf, size_t buflen, int flags);
ssize_t Read (int fd, void *buf, size_t buflen);
ssize_t Write (int fd, void *buf, size_t buflen);
int Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr* addr, socklen_t *len);	
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int Shutdown(int fd, int how); 
#endif
