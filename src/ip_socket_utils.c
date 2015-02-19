/* cranberry-server 
 * https://github.com/jahnf/cranberry-server
 * For licensing see LICENSE file or
 * https://github.com/jahnf/cranberry-server/blob/master/LICENSE
 */
 
#include <stdio.h>

#include "ip_socket_utils.h"
#include "settings.h"
#include "log.h"

SETLOGMODULENAME("ip_socket_utils");

/*
 * helper function to retrieve ip4/ip6 addresses
 * for opening sockets
 */
int server_getaddrinfo( thread_arg_t *args,
        int *v4avail, struct sockaddr_in *addrv4,
        int *v6avail, struct sockaddr_in6 *addrv6 )
{
    struct addrinfo hints = {0};    /* init struct */
    struct addrinfo *ai, *ai_loop, *aiv4 = NULL, *aiv6 = NULL;
    char portstr[32];
    int gaierr;
    char* hostname = NULL;
    server_settings_t *pSettings = args->pSettings;

    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if( v6avail ) *v6avail = 0;
    if( v4avail ) *v4avail = 0;
    sprintf( portstr, "%d", pSettings->port );

    if( (gaierr = getaddrinfo(hostname, portstr, &hints, &ai)) != 0 ) {
        LOG_CONSOLE( log_ERROR, "getaddrinfo error: %s", gai_strerror(gaierr) );
        return 0;
    }

    /* loop through address information, get first for v4/v6 */
    for( ai_loop = ai; ai_loop; ai_loop = ai_loop->ai_next ) {
        if( ai_loop->ai_family == AF_INET6 && aiv6 == NULL && pSettings->ipv6 ) {
            LOG( log_INFO, "IPv6 available" );
            aiv6 = ai_loop;
            if( v6avail ) *v6avail = 1;
            memset( addrv6, 0, sizeof(struct sockaddr_in6) );
            memcpy( addrv6 , aiv6->ai_addr, aiv6->ai_addrlen);
        }
        else if( ai_loop->ai_family == AF_INET && aiv4 == NULL ) {
            aiv4 = ai_loop;
            if( v4avail ) *v4avail = 1;
            memset( addrv4, 0, sizeof(struct sockaddr_in) );
            memcpy( addrv4 , aiv4->ai_addr, aiv4->ai_addrlen);
        }
        if( aiv4 && aiv6 ) break;
    }

    freeaddrinfo( ai );
    return 1;
}


/* MS Windows specific stuff */
#ifdef _WIN32
const char *inet_ntop_w32( int af, const void *src, 
                           char *dst, socklen_t cnt, const void *in_sa )
{
    if( in_sa != NULL ) {
        if( af == AF_INET )
            getnameinfo( (struct sockaddr *)in_sa, sizeof(struct sockaddr_in),
                            dst, cnt, NULL, 0, NI_NUMERICHOST);
        else if( af == AF_INET6 )
            getnameinfo( (struct sockaddr *)in_sa, sizeof(struct sockaddr_in6),
                            dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    else if (af == AF_INET) {
        struct sockaddr_in in = {0};
        in.sin_family = AF_INET;
        memcpy(&in.sin_addr, src, sizeof(struct in_addr));
        getnameinfo( (struct sockaddr *)&in, sizeof(struct sockaddr_in),
                        dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    else if (af == AF_INET6) {
        struct sockaddr_in6 in = {0};
        in.sin6_family = AF_INET6;
        memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
        getnameinfo( (struct sockaddr *)&in, sizeof(struct sockaddr_in6),
                        dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    return NULL;
}
#endif
