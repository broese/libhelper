/*! \file
 * Networking (TCP and UDP) functions
 */

#pragma once

#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef LH_NET_DEFAULT_BACKLOG
/*! Default backlog size parameter for listen().
 * Redefine prior to including this header file if necessary.*/
#define LH_NET_DEFAULT_BACKLOG 5
#endif

// TCP functions

/*! \brief Opens an IPv4/TCP listening socket on specific address and
 * port, with default parameters
 * \param ip IP address to bind to, in host byteorder
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
int lh_listen_tcp4(uint32_t ip, uint16_t port);

/*! \brief Open an IPv4/TCP listening socket on localhost and a specific port
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
#define lh_listen_tcp4_local(port) lh_listen_tcp4(INADDR_LOOPBACK,(port))

/*! \brief Open an IPv4/TCP listening socket on all interfaces and
 * a specific port
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
#define lh_listen_tcp4_any(port) lh_listen_tcp4(INADDR_ANY,(port))

/*! \brief Accept a IPv4/TCP connection from a remote client and set the
 * client socket to non-blocking mode.
 * \param ssock Listening TCP socket
 * \param cadr Pointer to a <tt>struct sockaddr_in</tt> where the client
 * address will be placed. Use NULL if not necessary.
 * \return File descriptor of the client socket.
 */
int lh_accept_tcp4(int ssock, struct sockaddr_in *cadr);

/*! \brief Open a IPv4/TCP connection to a remote address and port
 * \param ip IP address to connect to, in host byteorder
 * \param port Port number to connect to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
int lh_connect_tcp4(uint32_t ip, uint16_t port);

// UDP functions

/*! \brief Opens an IPv4/UDP socket on specific address and port
 * \param ip IP address to bind to, in host byteorder
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
int lh_listen_udp4(uint32_t ip, uint16_t port);

/*! \brief Opens an IPv4/UDP socket on localhost and a specific port
 * \param ip IP address to bind to, in host byteorder
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
#define lh_listen_udp4_local(port) lh_listen_udp4(INADDR_LOOPBACK,(port))

/*! \brief Opens an IPv4/UDP socket on all interfaces and a specific port
 * \param ip IP address to bind to, in host byteorder
 * \param port Port number to bind to, in host byteorder
 * \return Socket FD on success, -1 on failure. */
#define lh_listen_udp4_any(port) lh_listen_udp4(INADDR_ANY,(port))


// DNS functions
/*! \brief Get an IPv4 address from a hostname
 * \param hostname host name or dotted IP address as a string
 * \return IPv4 address as integer (in host byteorder),
 * 0xFFFFFFFF if failed to resolve
 */
uint32_t lh_dns_addr_ipv4(const char *hostname);

////////////////////////////////////////////////////////////////////////////////

int lh_net_nonblocking(int fd);
int lh_net_blocking(int fd);

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define listen_tcp4                     lh_listen_tcp4
#define listen_tcp4_local               lh_listen_tcp4_local
#define listen_tcp4_any                 lh_listen_tcp4_any
#define connect_tcp4                    lh_connect_tcp4
#define accept_tcp4                     lh_accept_tcp4
#define listen_udp4                     lh_listen_udp4
#define listen_udp4_local               lh_listen_udp4_local
#define listen_udp4_any                 lh_listen_udp4_any
#define dns_addr_ipv4                   lh_dns_addr_ipv4

#endif
