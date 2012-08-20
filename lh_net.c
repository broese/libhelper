// on Solaris, requires -lsocket -lnsl

#include "lh_debug.h"
#include "lh_files.h"
#include "lh_net.h"

static inline int sock_bind_ipv4(int s, uint32_t ip, uint16_t port) {
    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr.s_addr = htonl(ip)
    };
    
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) != 0)
        LH_ERROR(-1, "sock_bind_ipv4: bind failed");
    
    return 0;
}

// generic function to create listening TCP sockets for IPv4
int sock_server_ipv4_tcp(uint32_t ip, uint16_t port) {
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0) LH_ERROR(-1, "sock_server_ipv4_tcp: failed to create socket PF_INET/SOCK_STREAM/IPPROTO_TCP");

    int v = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));

    if (sock_bind_ipv4(s,ip,port)) return -1;

    if (listen(s, LH_NET_DEFAULT_BACKLOG)) LH_ERROR(-1, "sock_server_ipv4_tcp: listen failed");

    return s;
}

// generic function to create bound UDP sockets for IPv4
int sock_server_ipv4_udp(uint32_t ip, uint16_t port) {
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0) LH_ERROR(-1, "sock_server_ipv4_udp: failed to create socket PF_INET/SOCK_STREAM/IPPROTO_UDP");

    if (sock_bind_ipv4(s,ip,port)) return -1;

    return s;
}

