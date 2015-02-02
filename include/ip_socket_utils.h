#ifndef IP_SOCKET_UTILS__H_
#define IP_SOCKET_UTILS__H_

/* first some windows specific includes and defines */
#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0501
    #endif
    #include <direct.h>
    #include <Ws2tcpip.h>
    #include <winsock.h>
    #include <windows.h>

    #define socklen_t int

    const char *inet_ntop_w32(int af, const void *src, char *dst, socklen_t cnt, const void *in_sa);
    #define inet_ntop(a,b,c,d,e) inet_ntop_w32(a,b,c,d,e)
#else
    /* [ -- aix needs this */
    #include <sys/socket.h>
    #include <netinet/in.h>
    /* ] */
    #include <arpa/inet.h>
    #include <netdb.h>

    #define closesocket(s) close(s);
    #define inet_ntop(a,b,c,d,e) inet_ntop(a,b,c,d)
#endif

#include "webthread.h"

int server_getaddrinfo( thread_arg_t *args,
        int *v4avail, struct sockaddr_in *addrv4,
        int *v6avail, struct sockaddr_in6 *addrv6 );

#endif /* IP_SOCKET_UTILS__H_ */
