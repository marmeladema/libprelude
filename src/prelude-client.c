/*****
*
* Copyright (C) 2001, 2002, 2003 Yoann Vandoorselaere <yoann@prelude-ids.org>
* All Rights Reserved
*
* This file is part of the Prelude program.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "config.h"

#ifdef HAVE_SSL
#include "ssl.h"
#endif

#include "list.h"
#include "common.h"
#include "prelude-list.h"
#include "prelude-log.h"
#include "prelude-io.h"
#include "prelude-auth.h"
#include "prelude-inet.h"
#include "prelude-message.h"
#include "prelude-message-id.h"
#include "prelude-client.h"
#include "prelude-getopt.h"
#include "client-ident.h"
#include "prelude-path.h"




struct prelude_client {
        PRELUDE_LINKED_OBJECT;
        
        char *saddr;
        uint16_t sport;

        char *daddr;
        uint16_t dport;
        
        size_t sa_len;
        struct sockaddr *sa;
        
        /*
         * This pointer point to the object suitable
         * for writing to the Manager.
         */
        prelude_io_t *fd;

        uint8_t type;
        uint8_t state;
};




static void auth_error(prelude_client_t *client, const char *auth_kind) 
{
        log(LOG_INFO, "\n%s authentication failed. Please run :\n"
            "sensor-adduser --sensorname %s --uid %d --manager-addr %s\n"
            "program on the sensor host to create an account for this sensor.\n\n",
            auth_kind, prelude_get_sensor_name(), prelude_get_program_userid(), client->daddr);

        exit(1);
}




/*
 * Check if the tcp connection has been closed by peer
 * i.e if peer has sent a FIN tcp segment.
 *
 * It is important to call this function before writing on
 * a tcp socket, otherwise the write will succeed despite
 * the remote socket has been closed and next write will lead
 * to a broken pipe
 */
static int is_tcp_connection_still_established(prelude_io_t *pio)
{
        int pending, ret;
	struct pollfd pfd;
        
	pfd.events = POLLIN;
	pfd.fd = prelude_io_get_fd(pio);

	ret = poll(&pfd, 1, 0);
	if ( ret < 0 ) {
		log(LOG_ERR, "poll on tcp socket failed.\n");
		return -1;
	}

	if ( ret == 0 )
		return 0;

	if ( pfd.revents & POLLERR ) {
		log(LOG_ERR, "error polling tcp socket.\n");
		return -1;
	}

	if ( pfd.revents & POLLHUP ) {
		log(LOG_ERR, "connection hang up.\n");
		return -1;
	}

	if ( ! (pfd.revents & POLLIN) )
		return 0;

	/*
	 * Get the number of bytes to read
	 */
        pending = prelude_io_pending(pio);        
	if ( pending <= 0 ) {
		log(LOG_ERR, "connection has been closed by peer.\n");
		return -1;
	}

	return 0;
}




/*
 * Connect to the specified address in a generic manner
 * (can be Unix or Inet ).
 */
static int generic_connect(struct sockaddr *sa, socklen_t sa_len)
{
        int ret, sock, proto;

        if ( sa->sa_family == AF_UNIX ) 
                proto = 0;
        else
                proto = IPPROTO_TCP;
        
        sock = socket(sa->sa_family, SOCK_STREAM, proto);
	if ( sock < 0 ) {
                log(LOG_ERR, "error opening socket.\n");
		return -1;
	}
        
        ret = fcntl(sock, F_SETOWN, getpid());
        if ( ret < 0 ) {
                log(LOG_ERR, "couldn't set children to receive signal.\n");
                goto err;
        }

        ret = connect(sock, sa, sa_len);
	if ( ret < 0 ) {
                log(LOG_ERR,"error connecting socket.\n");
                goto err;
	}
        
	return sock;

  err:
        close(sock);
        return -1;
}




static int read_plaintext_authentication_result(prelude_client_t *client)
{
        int ret;
        void *buf;
        uint8_t tag;
        uint32_t dlen;
        prelude_msg_t *msg = NULL;
        prelude_msg_status_t status;

        do {
                status = prelude_msg_read(&msg, client->fd);
        } while ( status == prelude_msg_unfinished );
        
        if ( status != prelude_msg_finished ) {
                log(LOG_ERR, "error reading authentication result.\n");
                return -1;
        }

        ret = prelude_msg_get(msg, &tag, &dlen, &buf);
        prelude_msg_destroy(msg);
        
        if ( ret <= 0 ) {
                log(LOG_ERR, "error reading authentication result.\n");
                return -1;
        }
        
        if ( tag == PRELUDE_MSG_AUTH_SUCCEED ) {
                log(LOG_INFO, "- Plaintext authentication succeed with Prelude Manager.\n");
                return 0;
        }
        
        log(LOG_INFO, "- Plaintext authentication failed with Prelude Manager.\n");
        auth_error(client, "Plaintext");
        
        return -1;
}




static int handle_plaintext_connection(prelude_client_t *client, int sock) 
{
        int ret;
        size_t ulen, plen;
        char *user, *pass;
        prelude_msg_t *msg;
        char filename[256];

        prelude_get_auth_filename(filename, sizeof(filename));
        
        ret = prelude_auth_read_entry(filename, NULL, NULL, &user, &pass);
        if ( ret < 0 ) {
                /*
                 * the authentication file doesn't exist. Tell
                 * the user it have to create it and exit.
                 */
                auth_error(client, "Plaintext");
        }

        /*
         * create message containing plaintext authentication informations.
         */
        ulen = strlen(user) + 1;
        plen = strlen(pass) + 1;
        
        msg = prelude_msg_new(3, ulen + plen, PRELUDE_MSG_AUTH, 0);
        if ( ! msg ) {
                ret = -1;
                goto err;
        }

        prelude_msg_set(msg, PRELUDE_MSG_AUTH_PLAINTEXT, 0, NULL);
        prelude_msg_set(msg, PRELUDE_MSG_AUTH_USERNAME, ulen, user);
        prelude_msg_set(msg, PRELUDE_MSG_AUTH_PASSWORD, plen, pass);
        
        ret = prelude_msg_write(msg, client->fd);
        if ( ret <= 0 ) {
                log(LOG_ERR, "error sending plaintext authentication message.\n");
                ret = -1;
        }

        prelude_msg_destroy(msg);

  err:
        free(user);
        free(pass);

        return read_plaintext_authentication_result(client);
}



#ifdef HAVE_SSL

static int handle_ssl_connection(prelude_client_t *client, int sock) 
{
        int ret;
        SSL *ssl;
        prelude_msg_t *msg;
        static int ssl_initialized = 0;
        
        if ( ! ssl_initialized ) {
                ret = ssl_init_client();
                if ( ret < 0 ) {
                         /*
                          * SSL key / certificate doesn't exist. Tell the
                          * use how to create them and exit.
                          */
                        auth_error(client, "SSL");
                }
                
                ssl_initialized = 1;
        }
        
        msg = prelude_msg_new(1, 0, PRELUDE_MSG_AUTH, 0);
        if ( ! msg )
                return -1;

        prelude_msg_set(msg, PRELUDE_MSG_AUTH_SSL, 0, NULL);
        ret = prelude_msg_write(msg, client->fd);
        prelude_msg_destroy(msg);        
        
        if ( ret < 0 ) {
                log(LOG_ERR, "error sending SSL authentication message.\n");
                return -1;
        }
        
        ssl = ssl_connect_server(sock);
        if ( ! ssl ) {
                /*
                 * SSL authentication failed,
                 * tell the user how to setup SSL auth and exit.
                 */
                log(LOG_INFO, "- SSL authentication failed with Prelude Manager.\n");
                auth_error(client, "SSL");
        }

        log(LOG_INFO, "- SSL authentication succeed with Prelude Manager.\n");
        prelude_io_set_ssl_io(client->fd, ssl);

        return 0;
}

#endif




/*
 * Report server should send us a message containing
 * information about the kind of connecition it support.
 */
static int get_manager_setup(prelude_io_t *fd, int *have_ssl, int *have_plaintext) 
{
        int ret;
        void *buf;
        uint8_t tag;
        uint32_t dlen;
        prelude_msg_t *msg = NULL;
        prelude_msg_status_t status;

        do {
                status = prelude_msg_read(&msg, fd);
        } while ( status == prelude_msg_unfinished );
        
        if ( status != prelude_msg_finished ) {
                log(LOG_ERR, "error reading Manager configuration message (status=%d).\n", status);
                return -1;
        }

        if ( prelude_msg_get_tag(msg) != PRELUDE_MSG_AUTH ) {
                log(LOG_ERR, "Manager didn't sent us any authentication message.\n");
                return -1;
        }

        while ( (ret = prelude_msg_get(msg, &tag, &dlen, &buf)) > 0  ) {
                
                switch (tag) {

                case PRELUDE_MSG_AUTH_HAVE_SSL:
                        *have_ssl = 1;
                        break;

                case PRELUDE_MSG_AUTH_HAVE_PLAINTEXT:
                        *have_plaintext = 1;
                        break;

                default:
                        log(LOG_ERR, "Invalid authentication tag %d.\n", tag);
                        goto err;
                }
        }
        
 err:
        prelude_msg_destroy(msg);
        return ret;
}




static int start_inet_connection(prelude_client_t *client) 
{
        socklen_t len;
        struct sockaddr_in addr;
        int have_ssl = 0, have_plaintext = 0, sock, ret = -1;
        
        sock = generic_connect(client->sa, client->sa_len);
        if ( sock < 0 )
                return -1;

        /*
         * Get information about the connection,
         * because the sensor might want to know source addr/port used.
         */        
        len = sizeof(addr);

        ret = getsockname(sock, (struct sockaddr *) &addr, &len);
        if ( ret < 0 ) 
                log(LOG_ERR, "couldn't get connection informations.\n");
        else {
                client->saddr = strdup(inet_ntoa(addr.sin_addr));
                client->sport = ntohs(addr.sin_port);
        }
        
        prelude_io_set_sys_io(client->fd, sock);
        
        /*
         * get manager message telling what kind of connection it
         * support. Prefer SSL over plaintext if supported by both end.
         */
        ret = get_manager_setup(client->fd, &have_ssl, &have_plaintext);
        if ( ret < 0 ) {
                close(sock);
                return -1;
        }
                
#ifdef HAVE_SSL
        if ( have_ssl ) {
                ret = handle_ssl_connection(client, sock);
                goto end;
        }
#endif

        if ( have_plaintext ) {
                ret = handle_plaintext_connection(client, sock);
                goto end;
        } else {
                log(LOG_INFO, "couldn't agree on a protocol to use.\n");
                ret = -1;
        }
 end:
        if ( ret < 0 )
                close(sock);
        
        return ret;
}




static int start_unix_connection(prelude_client_t *client) 
{
        int ret, sock, have_ssl = 0, have_plaintext = 0;
        
        sock = generic_connect(client->sa, client->sa_len);
        if ( sock < 0 )
                return -1;
        
        prelude_io_set_sys_io(client->fd, sock);
        
        ret = get_manager_setup(client->fd, &have_ssl, &have_plaintext);
        if ( ret < 0 ) {
                close(sock);
                return -1;
        }

        if ( ! have_plaintext ) {
                log(LOG_INFO, "Unix connection used, but Manager report plaintext unavailable.\n");
                close(sock);
                return -1;
        }
        
        ret = handle_plaintext_connection(client, sock);
        if ( ret < 0 ) {
                close(sock);
                return -1;
        }
        
        return 0;
}




static int do_connect(prelude_client_t *client) 
{
        int ret;
        
        if ( client->sa->sa_family == AF_UNIX ) {
                log(LOG_INFO, "- Connecting to UNIX prelude Manager server.\n");
		ret = start_unix_connection(client);
        } else {
                log(LOG_INFO, "- Connecting to %s port %d prelude Manager server.\n",
                    client->daddr, client->dport);
                ret = start_inet_connection(client);
        }
        
        return ret;
}




static void close_client_fd(prelude_client_t *client) 
{
        client->state &= ~PRELUDE_CLIENT_CONNECTED;
        
        if ( ! (client->state & PRELUDE_CLIENT_OWN_FD) )
                return;
        
        prelude_io_close(client->fd);
        prelude_io_destroy(client->fd);
}



static void handle_connection_breakage(prelude_client_t *client)
{
        close_client_fd(client);
}




static int resolve_addr(prelude_client_t *client, const char *addr, uint16_t port) 
{
        int ret;
        prelude_addrinfo_t *ai, hints;
        char service[sizeof("00000")];

        memset(&hints, 0, sizeof(hints));
        
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        snprintf(service, sizeof(service), "%u", port);
        
        ret = prelude_inet_getaddrinfo(addr, service, &hints, &ai);        
        if ( ret < 0 ) {
                log(LOG_ERR, "couldn't resolve %s.\n", addr);
                return -1;
        }
        
        ret = prelude_inet_addr_is_loopback(ai->ai_family, prelude_inet_sockaddr_get_inaddr(ai->ai_addr));        
        if ( ret == 0 ) {                
                ai->ai_family = AF_UNIX;
                ai->ai_addrlen = sizeof(struct sockaddr_un);
        }

        client->sa = malloc(ai->ai_addrlen);
        if ( ! client->sa ) {
                log(LOG_ERR, "memory exhausted.\n");
                prelude_inet_freeaddrinfo(ai);
                return -1;
        }

        client->sa_len = ai->ai_addrlen;
        client->sa->sa_family = ai->ai_family;

        if ( ai->ai_family != AF_UNIX )
                memcpy(client->sa, ai->ai_addr, ai->ai_addrlen);
        else {
                struct sockaddr_un *un = (struct sockaddr_un *) client->sa;
                prelude_get_socket_filename(un->sun_path, sizeof(un->sun_path), port);
        }
                
        prelude_inet_freeaddrinfo(ai);

        return 0;
}



/*
 * function called back when a connection timer expire.
 */

void prelude_client_destroy(prelude_client_t *client) 
{
	free(client->sa);
        close_client_fd(client);

        if ( client->saddr )
                free(client->saddr);
        
        free(client->daddr);
        free(client);
}




prelude_client_t *prelude_client_new(const char *addr, uint16_t port) 
{
        int ret;
        prelude_client_t *new;
        
        
        signal(SIGPIPE, SIG_IGN);

        new = malloc(sizeof(*new));
        if (! new )
                return NULL;
        
        ret = resolve_addr(new, addr, port);
        if ( ret < 0 ) {
                log(LOG_ERR, "couldn't resolve %s.\n", addr);
                return NULL;
        }
        
        new->saddr = NULL;
        new->sport = 0;
        new->daddr = strdup(addr);
        new->dport = port;
        new->type = PRELUDE_CLIENT_TYPE_OTHER;
        new->state = 0;
                        
        return new;
}




void prelude_client_set_fd(prelude_client_t *client, prelude_io_t *fd) 
{
        close_client_fd(client);
        client->fd = fd;

        /*
         * The caller is responssible for fd closing/destruction.
         * Unless it specify otherwise using prelude_client_set_state().
         */
        client->state &= ~PRELUDE_CLIENT_OWN_FD;
}




int prelude_client_connect(prelude_client_t *client) 
{
        int ret;
        prelude_msg_t *msg;

        client->state &= ~PRELUDE_CLIENT_CONNECTED;
        
        client->fd = prelude_io_new();
        if ( ! client->fd ) 
                return -1;
        
        ret = do_connect(client);
        if ( ret < 0 ) {
                prelude_io_destroy(client->fd);
                return -1;
        }
        
        msg = prelude_msg_new(1, sizeof(client->type), PRELUDE_MSG_CLIENT_TYPE, 0);
        if ( ! msg )
                goto err;

        prelude_msg_set(msg, client->type, 0, NULL);
        ret = prelude_msg_write(msg, client->fd);
        prelude_msg_destroy(msg);
        
        if ( ret < 0 ) 
                goto err;
                
        ret = prelude_client_ident_send(client->fd, client->type);
        if ( ret < 0 )
                goto err;

        msg = prelude_option_wide_get_msg();
        if ( msg ) {
                ret = prelude_msg_write(msg, client->fd);
                if ( ret < 0 )
                        goto err;
        }
        
        client->state |= PRELUDE_CLIENT_OWN_FD;
        client->state |= PRELUDE_CLIENT_CONNECTED;
        
        return ret;
        
 err:
        prelude_io_close(client->fd);
        prelude_io_destroy(client->fd);
        
        return ret;
}




void prelude_client_close(prelude_client_t *client) 
{
        close_client_fd(client);
}




int prelude_client_send_msg(prelude_client_t *client, prelude_msg_t *msg) 
{
        ssize_t ret;
        
        if ( ! (client->state & PRELUDE_CLIENT_CONNECTED) )
                return -1;

        ret = prelude_msg_write(msg, client->fd);
        if ( ret < 0 || (ret = is_tcp_connection_still_established(client->fd)) < 0 ) {
                log(LOG_ERR, "could not send message to Manager.\n");
                handle_connection_breakage(client);
        }
        
        return ret;
}



/**
 * prelude_client_get_fd:
 * @client: Pointer to a client object.
 *
 * Returns: A pointer to a #prelude_io_t object used for the
 * communication with the client.
 */
prelude_io_t *prelude_client_get_fd(prelude_client_t *client) 
{
        return client->fd;
}




/**
 * prelude_client_get_saddr:
 * @client: Pointer to a client object.
 *
 *
 * Returns: the source address used to connect, or NULL
 * if an error occured.
 */
const char *prelude_client_get_saddr(prelude_client_t *client) 
{
        return client->saddr;
}



/**
 * prelude_client_get_daddr:
 * @client: Pointer to a client object.
 *
 *
 * Returns: the destination address used to connect.
 */
const char *prelude_client_get_daddr(prelude_client_t *client) 
{
        return client->daddr;
}



/**
 * prelude_client_get_sport:
 * @client: Pointer to a client object.
 *
 *
 * Returns: the source port used to connect.
 */
uint16_t prelude_client_get_sport(prelude_client_t *client) 
{
        return client->sport;
}



/**
 * prelude_client_get_sport:
 * @client: Pointer to a client object.
 *
 *
 * Returns: the destination port used to connect.
 */
uint16_t prelude_client_get_dport(prelude_client_t *client) 
{
        return client->dport;
}



/**
 * prelude_client_is_alive:
 * @client: Pointer to a client object.
 *
 * Returns: 0 if the connection associated with @client is alive,
 * -1 if it is not.
 */
int prelude_client_is_alive(prelude_client_t *client) 
{
        return (client->state & PRELUDE_CLIENT_CONNECTED) ? 0 : -1;
}



void prelude_client_set_type(prelude_client_t *client, int type) 
{
        client->type = type;
}



int prelude_client_get_type(prelude_client_t *client) 
{
        return client->type;
}




void prelude_client_set_state(prelude_client_t *client, int state) 
{
        client->state = state;
}




int prelude_client_get_state(prelude_client_t *client)
{
        return client->state;
}




ssize_t prelude_client_forward(prelude_client_t *client, prelude_io_t *src, size_t count) 
{
        ssize_t ret;

        if ( ! (client->state & PRELUDE_CLIENT_CONNECTED) )
                return -1;
        
        ret = prelude_io_forward(client->fd, src, count);
        if ( ret < 0 ) {
                log(LOG_ERR, "error forwarding message to Manager.\n");
                handle_connection_breakage(client);
        }

        return ret;
}

