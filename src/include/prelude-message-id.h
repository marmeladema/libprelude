/*****
*
* Copyright (C) 2001, 2002 Yoann Vandoorselaere <yoann@mandrakesoft.com>
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

#ifndef _LIBPRELUDE_PRELUDE_MESSAGE_ID_H
#define _LIBPRELUDE_PRELUDE_MESSAGE_ID_H

/*
 * Top level message tag (to be used with prelude_msg_new()).
 */
#define PRELUDE_MSG_IDMEF       0
#define PRELUDE_MSG_OPTION_LIST 1
#define PRELUDE_MSG_OPTION_SET  2
#define PRELUDE_MSG_ID          3
#define PRELUDE_MSG_AUTH        4

/*
 * PRELUDE_MSG_ID submessage
 */
#define PRELUDE_MSG_ID_DECLARE  0


/*
 * PRELUDE_MSG_AUTH submessage
 */
#define PRELUDE_MSG_AUTH_HAVE_SSL       0
#define PRELUDE_MSG_AUTH_HAVE_PLAINTEXT 1

#define PRELUDE_MSG_AUTH_SSL       2
#define PRELUDE_MSG_AUTH_PLAINTEXT 3
#define PRELUDE_MSG_AUTH_USERNAME  4
#define PRELUDE_MSG_AUTH_PASSWORD  5

#define PRELUDE_MSG_AUTH_SUCCEED   6
#define PRELUDE_MSG_AUTH_FAILED    7
#define PRELUDE_MSG_AUTH_EXIST     8

#endif /* _LIBPRELUDE_PRELUDE_MESSAGE_ID_H */
