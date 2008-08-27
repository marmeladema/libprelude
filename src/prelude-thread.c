/*****
*
* Copyright (C) 2005 PreludeIDS Technologies. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoann.v@prelude-ids.com>
*
* This file is part of the Prelude library.
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

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "glthread/tls.h"

#include "prelude.h"
#include "prelude-inttypes.h"
#include "prelude-thread.h"
#include "prelude-log.h"


static gl_tls_key_t thread_error_key;
static prelude_bool_t use_thread = FALSE, need_init = TRUE;



static void thread_error_key_destroy(void *value)
{
        free(value);
}



/*
 *
 */
int prelude_thread_init(void *nil)
{
        gl_tls_key_init(thread_error_key, thread_error_key_destroy);
        return 0;
}



/*
 *
 */
void _prelude_thread_deinit(void)
{
        char *previous;

        previous = gl_tls_get(thread_error_key);
        if ( previous )
                free(previous);

        gl_tls_key_destroy(thread_error_key);
}



int _prelude_thread_set_error(const char *error)
{
        char *previous;

        previous = gl_tls_get(thread_error_key);
        if ( previous )
                free(previous);

        gl_tls_set(thread_error_key, strdup(error));

        return 0;
}



const char *_prelude_thread_get_error(void)
{
        return gl_tls_get(thread_error_key);
}

