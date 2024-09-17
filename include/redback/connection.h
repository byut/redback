#ifndef REDBACK_CONNECTION_H
#define REDBACK_CONNECTION_H

/* # */

struct sockaddr;
struct bufferevent;

/* # */

///
/// @brief ...
///
struct redback_connection {
    struct bufferevent *bev;
    struct sockaddr *addr;
    int addrlen;
};

/* # */

#endif
