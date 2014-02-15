// on Solaris, requires -lsocket -lnsl

#define LH_DECLARE_SHORT_NAMES

#include "lh_buffers.h"
#include "lh_debug.h"
#include "lh_files.h"
#include "lh_net.h"

#include <unistd.h>
#include <fcntl.h>

static inline int sock_bind_ipv4(int s, uint32_t ip, uint16_t port) {
    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr.s_addr = htonl(ip)
    };
    
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) != 0)
        LH_ERROR(-1, "%s: bind failed", __func__);
    
    return 0;
}

// generic function to create listening TCP sockets for IPv4
int lh_listen_tcp4(uint32_t ip, uint16_t port) {
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
        LH_ERROR(-1, "%s: failed to create socket", __func__);

    int v = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));

    if (sock_bind_ipv4(s,ip,port))
        return -1;

    if (listen(s, LH_NET_DEFAULT_BACKLOG))
        LH_ERROR(-1, "%s: listen failed", __func__);

    return s;
}

// generic function to create bound UDP sockets for IPv4
int lh_listen_udp4(uint32_t ip, uint16_t port) {
    int s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
        LH_ERROR(-1, "%s: failed to create socket", __func__);

    if (sock_bind_ipv4(s,ip,port)) return -1;

    return s;
}

////////////////////////////////////////////////////////////////////////////////

int lh_accept_tcp4(int ssock, struct sockaddr_in *cadr) {
    socklen_t sa_size = sizeof(*cadr);
    int cl = accept(ssock, (struct sockaddr *)cadr, cadr ? &sa_size : NULL);
    if (cl<0) LH_ERROR(-1, "Failed to accept on socket %d",ssock);
    if (fcntl(cl, F_SETFL, O_NONBLOCK) < 0) {
        close(cl);
        LH_ERROR(-1,"Failed to set O_NONBLOCK on client socket");
    }
    return cl;
}

////////////////////////////////////////////////////////////////////////////////

// make a TCP connection to a remote machine
int lh_connect_tcp4(uint32_t ip, uint16_t port) {
    int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
        LH_ERROR(-1, "%s: failed to create socket", __func__);

    struct sockaddr_in sa = {
        .sin_family = AF_INET,
        .sin_port   = htons(port),
        .sin_addr.s_addr = htonl(ip)
    };

    if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        LH_ERROR(-1, "%s: bind failed", __func__);
    }

    return s;
}

////////////////////////////////////////////////////////////////////////////////

//FIXME: come up with a better idea to return an error
uint32_t lh_dns_addr_ipv4(const char *hostname) {
    struct addrinfo hints;
    CLEAR(hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;

    int res = getaddrinfo(hostname, NULL, &hints, &result);

    if (res) {
        LH_ERROR(0xffffffff, "%s: cannot resolve hostname %s : %s",
                 __func__,hostname,gai_strerror(res));
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)result[0].ai_addr;
    uint32_t addr = ntohl(sin->sin_addr.s_addr);
    freeaddrinfo(result);

    return addr;
}
