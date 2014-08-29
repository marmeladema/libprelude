/*****
*
* Copyright (C) 2005-2014 CS-SI. All Rights Reserved.
* Author: Yoann Vandoorselaere <yoannv@prelude-ids.com>
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

%module PreludeEasy
%feature("nothread", "1");

%include "pystrings.swg"
%include "std_string.i"
%include "std_vector.i"
%include "exception.i"

%{
#include <list>
#include <sstream>

#include "prelude.hxx"
#include "prelude-log.hxx"
#include "prelude-error.hxx"
#include "prelude-connection.hxx"
#include "prelude-connection-pool.hxx"
#include "prelude-client-profile.hxx"
#include "prelude-client.hxx"
#include "prelude-client-easy.hxx"
#include "idmef-criteria.hxx"
#include "idmef-value.hxx"
#include "idmef-path.hxx"
#include "idmef-time.hxx"
#include "idmef.hxx"

using namespace Prelude;
%}


typedef char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;

typedef int int32_t;
typedef unsigned int uint32_t;

typedef long long int64_t;
typedef unsigned long long uint64_t;

%ignore prelude_error_t;
typedef signed int prelude_error_t;

%exception {
        try {
                $action
        } catch(Prelude::PreludeError &e) {
                SWIG_exception(SWIG_RuntimeError, e.what());
                SWIG_fail;
        }
}


#ifdef SWIGPERL
%include perl/libpreludecpp-perl.i
#endif

#ifdef SWIGPYTHON
%include python/libpreludecpp-python.i
#endif

#ifdef SWIGRUBY
%include ruby/libpreludecpp-ruby.i
#endif

#ifdef SWIGLUA
%include lua/libpreludecpp-lua.i
#endif


%ignore operator <<(std::ostream &os, const Prelude::IDMEF &idmef);
%ignore operator >>(std::istream &is, const Prelude::IDMEF &idmef);


%template() std::vector<std::string>;
%template() std::vector<Prelude::IDMEFValue>;
%template() std::vector<Prelude::Connection>;

%extend Prelude::IDMEF {
        Prelude::IDMEFValue Get(const char *path) {
                Prelude::IDMEFValue value;
                Prelude::IDMEFPath ipath = Prelude::IDMEFPath(*self, path);

                value = ipath.Get(*self);
                if ( value.IsNull() && ipath.IsAmbiguous() ) {
                        std::vector<Prelude::IDMEFValue> v;
                        return Prelude::IDMEFValue(v);
                }

                return value;
        }
}

%extend Prelude::IDMEFPath {
        Prelude::IDMEFValue Get(Prelude::IDMEF &message) {
                Prelude::IDMEFValue value;

                value = self->Get(message);
                if ( value.IsNull() && self->IsAmbiguous() ) {
                        std::vector<Prelude::IDMEFValue> v;
                        return Prelude::IDMEFValue(v);
                }

                return value;
        }
}

%ignore Prelude::IDMEF::Get;
%ignore Prelude::IDMEFPath::Get;


%fragment("IDMEFValue_to_SWIG", "header", fragment="IDMEFValueList_to_SWIG", fragment="SWIG_From_float") {


int IDMEFValue_to_SWIG(const Prelude::IDMEFValue &result, TARGET_LANGUAGE_OUTPUT_TYPE ret)
{
        std::stringstream s;
        idmef_value_t *value = result;
        Prelude::IDMEFValue::IDMEFValueTypeEnum type = result.GetType();

        if ( type == Prelude::IDMEFValue::TYPE_STRING ) {
                prelude_string_t *str = idmef_value_get_string(value);
                *ret = SWIG_FromCharPtrAndSize(prelude_string_get_string(str), prelude_string_get_len(str));
        }

        else if ( type == Prelude::IDMEFValue::TYPE_INT8 )
                *ret = SWIG_From_int(idmef_value_get_int8(value));

        else if ( type == Prelude::IDMEFValue::TYPE_UINT8 )
                *ret = SWIG_From_unsigned_SS_int(idmef_value_get_uint8(value));

        else if ( type == Prelude::IDMEFValue::TYPE_INT16 )
                *ret = SWIG_From_int(idmef_value_get_int16(value));

        else if ( type == Prelude::IDMEFValue::TYPE_UINT16 )
                *ret = SWIG_From_unsigned_SS_int(idmef_value_get_uint16(value));

        else if ( type == Prelude::IDMEFValue::TYPE_INT32 )
                *ret = SWIG_From_int(idmef_value_get_int32(value));

        else if ( type == Prelude::IDMEFValue::TYPE_UINT32 )
                *ret = SWIG_From_unsigned_SS_int(idmef_value_get_uint32(value));

        else if ( type == Prelude::IDMEFValue::TYPE_INT64 )
                *ret = SWIG_From_long_SS_long(idmef_value_get_int64(value));

        else if ( type == Prelude::IDMEFValue::TYPE_UINT64 )
                *ret = SWIG_From_unsigned_SS_long_SS_long(idmef_value_get_uint64(value));

        else if ( type == Prelude::IDMEFValue::TYPE_FLOAT )
                *ret = SWIG_From_float(idmef_value_get_float(value));

        else if ( type == Prelude::IDMEFValue::TYPE_DOUBLE )
                *ret = SWIG_From_double(idmef_value_get_double(value));

        else if ( type == Prelude::IDMEFValue::TYPE_ENUM ) {
                const char *s = idmef_class_enum_to_string(idmef_value_get_class(value), idmef_value_get_enum(value));
                *ret = SWIG_FromCharPtr(s);
        }

        else if ( type == Prelude::IDMEFValue::TYPE_TIME ) {
                Prelude::IDMEFTime t = result;
                *ret = SWIG_NewPointerObj(new Prelude::IDMEFTime(t), $descriptor(Prelude::IDMEFTime *), 1);
        }

        else if ( type == Prelude::IDMEFValue::TYPE_LIST )
                *ret = IDMEFValueList_to_SWIG(result);

        else if ( type == Prelude::IDMEFValue::TYPE_DATA ) {
                idmef_data_t *d = idmef_value_get_data(value);
                idmef_data_type_t t = idmef_data_get_type(d);

                if ( t == IDMEF_DATA_TYPE_CHAR ||
                     t == IDMEF_DATA_TYPE_BYTE || t == IDMEF_DATA_TYPE_BYTE_STRING )
                        *ret = SWIG_FromCharPtrAndSize((const char *)idmef_data_get_data(d), idmef_data_get_len(d));

                else if ( t == IDMEF_DATA_TYPE_CHAR_STRING )
                        *ret = SWIG_FromCharPtrAndSize((const char *)idmef_data_get_data(d), idmef_data_get_len(d) - 1);

                else if ( t == IDMEF_DATA_TYPE_FLOAT )
                        *ret = SWIG_From_float(idmef_data_get_float(d));

                else if ( t == IDMEF_DATA_TYPE_UINT32 )
                        *ret = SWIG_From_unsigned_SS_int(idmef_data_get_uint32(d));

                else if ( t == IDMEF_DATA_TYPE_UINT64 )
                        *ret = SWIG_From_unsigned_SS_long_SS_long(idmef_data_get_uint64(d));
        }

        else if ( type == Prelude::IDMEFValue::TYPE_CLASS ) {
                idmef_object_t *obj = (idmef_object_t *) idmef_value_get_object(value);
                *ret = SWIG_NewPointerObj(new Prelude::IDMEF(idmef_object_ref(obj)), $descriptor(Prelude::IDMEF *), 1);
        }

        else return -1;

        return 0;
}
}

%ignore Prelude::IDMEFValue::operator const char*() const;
%ignore Prelude::IDMEFValue::operator std::vector<IDMEFValue>() const;
%ignore Prelude::IDMEFValue::operator Prelude::IDMEFTime() const;
%ignore Prelude::IDMEFValue::operator int8_t() const;
%ignore Prelude::IDMEFValue::operator uint8_t() const;
%ignore Prelude::IDMEFValue::operator int16_t() const;
%ignore Prelude::IDMEFValue::operator uint16_t() const;
%ignore Prelude::IDMEFValue::operator int32_t() const;
%ignore Prelude::IDMEFValue::operator uint32_t() const;
%ignore Prelude::IDMEFValue::operator int64_t() const;
%ignore Prelude::IDMEFValue::operator uint64_t() const;
%ignore Prelude::IDMEFValue::operator float() const;
%ignore Prelude::IDMEFValue::operator double() const;

/*
 * Force SWIG to use the IDMEFValue * version of the Set() function,
 * so that the user might provide NULL IDMEFValue.
 */
%ignore Prelude::IDMEF::Set(char const *, int8_t);
%ignore Prelude::IDMEF::Set(char const *, uint8_t);
%ignore Prelude::IDMEF::Set(char const *, Prelude::IDMEFValue &value);
%ignore Prelude::IDMEFPath::Set(Prelude::IDMEF &, int8_t);
%ignore Prelude::IDMEFPath::Set(Prelude::IDMEF &, uint8_t);
%ignore Prelude::IDMEFPath::Set(Prelude::IDMEF &, Prelude::IDMEFValue &);

%ignore idmef_path_t;
%ignore idmef_criteria_t;
%ignore prelude_client_t;
%ignore prelude_client_profile_t;
%ignore prelude_connection_t;
%ignore prelude_connection_pool_t;
%ignore operator prelude_connection_t *();
%ignore operator prelude_connection_pool_t *();
%ignore operator idmef_message_t *() const;
%ignore operator idmef_time_t *() const;
%ignore operator idmef_value_t *() const;
%ignore operator prelude_client_profile_t *() const;


/*
 * We need to unlock the interpreter lock before calling certain methods
 * because they might acquire internal libprelude mutex that may also be
 * acquired undirectly through the libprelude asynchronous stack.
 *
 * [Thread 2]: Libprelude async stack
 * -> Lock internal mutexX
 *    -> prelude_log()
 *       -> SWIG/C log callback
 *          -> Wait on Interpreter lock [DEADLOCK]
 *             -> Python logging callback (never called)
 *
 * [Thread 1] ConnectionPool::Recv()
 *  -> Acquire Interpreter lock
 *      *** At this time, thread 2 lock internal mutexX
 *      -> Wait on internal mutexX [DEADLOCK]
 *
 * In this situation, [Thread 1] hold the Interpreter lock and is
 * waiting on mutexX, which itself cannot be released by [Thread 2]
 * until [Thread 1] unlock the Interpreter lock.
 *
 * One rule to prevent deadlock is to always acquire mutex in the same
 * order. We thus need to make sure the interpreter lock is released
 * before calling C++ method that are susceptible to lock a mutex that
 * is also triggered from the asynchronous interface.
 *
 * Note that we are not releasing the Interpreter lock in all C++ call,
 * because it come at a performance cost, so we only try to do it when
 * needed.
 */

#ifdef SWIG_COMPILE_LIBPRELUDE
%include prelude.hxx
%include prelude-client-profile.hxx

%feature("nothread", "0");
%include prelude-connection.hxx
%include prelude-connection-pool.hxx
%include prelude-client.hxx
%feature("nothread", "1");

%include prelude-log.hxx
%include prelude-error.hxx
%include prelude-client-easy.hxx
%include idmef-criteria.hxx
%include idmef-value.hxx
%include idmef-path.hxx
%include idmef-time.hxx
%include idmef.hxx
#endif
