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

#ifndef _LIBPRELUDE_PRELUDE_AUTH_H
#define _LIBPRELUDE_PRELUDE_AUTH_H

int prelude_auth_create_account(const char *filename, char **user, char **pass,
                                int crypted, uid_t uid);

int prelude_auth_create_account_noprompt(const char *filename, const char *user,
                                         const char *pass, int crypted, uid_t uid);

int prelude_auth_check(const char *authfile, const char *user, const char *pass);


/*
 * theses constant should only be used with
 * prelude_auth_read_entry() return value.
 */
#define user_does_not_exist      -2
#define password_does_not_match  -3

int prelude_auth_read_entry(const char *authfile, const char *wanted_user,
                            const char *wanted_pass, char **user, char **pass);

#endif /* _LIBPRELUDE_PRELUDE_AUTH_H */
