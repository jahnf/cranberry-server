/** @file windows_utils.c 
 * Replacements for missing functions on MS Windows. */

#ifdef _WIN32

const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
    if (af == AF_INET) {
        struct sockaddr_in in;
        memset(&in, 0, sizeof(in));
        in.sin_family = AF_INET;
        memcpy(&in.sin_addr, src, sizeof(struct in_addr));
        getnameinfo((struct sockaddr *)&in, sizeof(struct
        sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    else if (af == AF_INET6) {
        struct sockaddr_in6 in;
        memset(&in, 0, sizeof(in));
        in.sin6_family = AF_INET6;
        memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
        getnameinfo((struct sockaddr *)&in, sizeof(struct
        sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
        return dst;
    }
    return NULL;
}

int inet_pton(int af, const char *src, void *dst) 
{
    struct addrinfo hints, *res, *ressave;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = af;

    if (getaddrinfo(src, NULL, &hints, &res) != 0) {
        dolog(LOG_ERR, "Couldn't resolve host %s\n", src);
        return -1;
    }

    ressave = res;

    while (res) {
        memcpy(dst, res->ai_addr, res->ai_addrlen);
        res = res->ai_next;
    }

    freeaddrinfo(ressave);
    return 0;
}
#endif
