#pragma once

#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LH_NET_DEFAULT_BACKLOG 5

// TCP functions

int sock_server_ipv4_tcp(uint32_t ip, uint16_t port);
#define sock_server_ipv4_tcp_local(port) sock_server_ipv4_tcp(INADDR_LOOPBACK,(port))
#define sock_server_ipv4_tcp_any(port) sock_server_ipv4_tcp(INADDR_ANY,(port))

// UDP functions

int sock_server_ipv4_udp(uint32_t ip, uint16_t port);
#define sock_server_ipv4_udp_local(port) sock_server_ipv4_udp(INADDR_LOOPBACK,(port))
#define sock_server_ipv4_udp_any(port) sock_server_ipv4_udp(INADDR_ANY,(port))

// FILE * accept_connection(int ssock, uint32_t);

