/*****
*
* Copyright (C) 2004 Yoann Vandoorselaere <yoann@prelude-ids.org>
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

#ifndef _LIBPRELUDE_PRELUDE_CLIENT_H
#define _LIBPRELUDE_PRELUDE_CLIENT_H

typedef enum {
        PRELUDE_CLIENT_EXIT_STATUS_SUCCESS = 0,
        PRELUDE_CLIENT_EXIT_STATUS_FAILURE = -1
} prelude_client_exit_status_t;


typedef enum {
        PRELUDE_CLIENT_FLAGS_ASYNC_SEND  = 0x01,
        PRELUDE_CLIENT_FLAGS_ASYNC_TIMER = 0x02
} prelude_client_flags_t;


typedef struct prelude_client prelude_client_t;


#include "prelude-client-profile.h"
#include "prelude-ident.h"
#include "prelude-connection.h"
#include "prelude-connection-mgr.h"
#include "idmef.h"


prelude_ident_t *prelude_client_get_unique_ident(prelude_client_t *client);

void prelude_client_set_manager_list(prelude_client_t *client, prelude_connection_mgr_t *mgrlist);

prelude_connection_mgr_t *prelude_client_get_manager_list(prelude_client_t *client);

int prelude_client_start(prelude_client_t *client);

int prelude_client_new(prelude_client_t **client,
                       prelude_connection_capability_t capability,
                       const char *profile, const char *config);

idmef_analyzer_t *prelude_client_get_analyzer(prelude_client_t *client);

prelude_client_flags_t prelude_client_get_flags(prelude_client_t *client);

prelude_connection_capability_t prelude_client_get_capability(prelude_client_t *client);

void prelude_client_send_msg(prelude_client_t *client, prelude_msg_t *msg);

void prelude_client_set_heartbeat_cb(prelude_client_t *client, void (*cb)(prelude_client_t *client, idmef_message_t *hb));

void prelude_client_send_idmef(prelude_client_t *client, idmef_message_t *msg);

void prelude_client_destroy(prelude_client_t *client, prelude_client_exit_status_t status);

int prelude_client_set_flags(prelude_client_t *client, prelude_client_flags_t flags);

const char *prelude_client_get_config_filename(prelude_client_t *client);

void prelude_client_print_setup_error(prelude_client_t *client);

prelude_bool_t prelude_client_is_setup_needed(prelude_client_t *client, int error);

prelude_client_profile_t *prelude_client_get_profile(prelude_client_t *client);

int _prelude_client_register_options(void);

#endif
