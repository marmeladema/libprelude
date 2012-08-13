/*****
*
* Copyright (C) 2008-2012 CS-SI. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoann@prelude-ids.com>
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
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*****/

#ifndef _LIBPRELUDE_IDMEF_TIME_HXX
#define _LIBPRELUDE_IDMEF_TIME_HXX

#include "prelude.h"

namespace Prelude {
        class IDMEFTime {
            protected:
                idmef_time_t *_time;

            public:
                IDMEFTime();
                IDMEFTime(idmef_time_t *time);
                IDMEFTime(const time_t *time);
                IDMEFTime(const char *string);
                IDMEFTime(const struct timeval *tv);
                IDMEFTime(const IDMEFTime &value);
                ~IDMEFTime();

                void Set();
                void Set(const time_t *time);
                void Set(const char *string);
                void Set(const struct timeval *tv);
                void SetSec(uint32_t sec);
                void SetUSec(uint32_t usec);
                void SetGmtOffset(int32_t gmtoff);

                uint32_t GetSec() const;
                uint32_t GetUSec() const;
                int32_t GetGmtOffset() const;

                IDMEFTime Clone();
                const std::string ToString() const;

                operator int() const;
                operator long() const;
                operator double() const;
                operator const std::string() const;
                operator idmef_time_t *() const;

                bool operator != (const IDMEFTime &time);
                bool operator >= (const IDMEFTime &time);
                bool operator <= (const IDMEFTime &time);
                bool operator == (const IDMEFTime &time);
                bool operator > (const IDMEFTime &time);
                bool operator < (const IDMEFTime &time);

                IDMEFTime & operator = (const IDMEFTime &p);
        };
};

#endif
