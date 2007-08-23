# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2004-2007 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects using CVS, this file can be treated like other built files.


# This macro should be invoked from ./configure.in, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  m4_pattern_allow([^gl_LIBOBJS$])dnl a variable
  m4_pattern_allow([^gl_LTLIBOBJS$])dnl a variable
  AC_REQUIRE([AC_PROG_RANLIB])
  AC_REQUIRE([AC_GNU_SOURCE])
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  AC_REQUIRE([AC_FUNC_FSEEKO])
])

# This macro should be invoked from ./configure.in, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  AM_CONDITIONAL([GL_COND_LIBTOOL], [true])
  gl_cond_libtool=true
  gl_source_base='libmissing'
changequote(,)dnl
LTALLOCA=`echo "$ALLOCA" | sed 's/\.[^.]* /.lo /g;s/\.[^.]*$/.lo/'`
changequote([, ])dnl
AC_SUBST([LTALLOCA])
  gl_FUNC_ALLOCA
  gl_HEADER_ARPA_INET
  AC_PROG_MKDIR_P
  gl_FLOAT_H
  gl_FUNC_FSEEKO
  gl_STDIO_MODULE_INDICATOR([fseeko])
  gl_FUNC_FTW
  gl_GETADDRINFO
  gl_FUNC_GETDELIM
  gl_STDIO_MODULE_INDICATOR([getdelim])
  gl_FUNC_GETLINE
  gl_STDIO_MODULE_INDICATOR([getline])
  gl_FUNC_GETPASS
  AC_SUBST([LIBINTL])
  AC_SUBST([LTLIBINTL])
  gl_FUNC_GETTIMEOFDAY
  gl_INET_NTOP
  gl_LOCALCHARSET
  gl_FUNC_LSEEK
  gl_UNISTD_MODULE_INDICATOR([lseek])
  AC_FUNC_MALLOC
  gl_MALLOCA
  gl_FUNC_MEMMEM
  gl_STRING_MODULE_INDICATOR([memmem])
  gl_MINMAX
  gl_FUNC_MKTIME
  gl_HEADER_NETINET_IN
  AC_PROG_MKDIR_P
  gl_PATHMAX
  gl_FUNC_POLL
  gl_REGEX
  gl_SIZE_MAX
  gl_FUNC_SNPRINTF
  gl_STDIO_MODULE_INDICATOR([snprintf])
  gl_TYPE_SOCKLEN_T
  gt_TYPE_SSIZE_T
  AM_STDBOOL_H
  gl_STDINT_H
  gl_STDIO_H
  gl_STRCASE
  gl_FUNC_STRCASESTR
  gl_STRING_MODULE_INDICATOR([strcasestr])
  gl_FUNC_STRDUP
  gl_STRING_MODULE_INDICATOR([strdup])
  gl_HEADER_STRING_H
  gl_FUNC_STRNDUP
  gl_STRING_MODULE_INDICATOR([strndup])
  gl_FUNC_STRNLEN
  gl_STRING_MODULE_INDICATOR([strnlen])
  gl_FUNC_STRPBRK
  gl_STRING_MODULE_INDICATOR([strpbrk])
  gl_FUNC_STRPTIME
  gl_FUNC_STRSEP
  gl_STRING_MODULE_INDICATOR([strsep])
  gl_HEADER_SYS_SELECT
  AC_PROG_MKDIR_P
  gl_HEADER_SYS_SOCKET
  AC_PROG_MKDIR_P
  gl_HEADER_SYS_TIME_H
  AC_PROG_MKDIR_P
  gl_HEADER_TIME_H
  gl_TIME_R
  gl_FUNC_TIMEGM
  gl_UNISTD_H
  gl_FUNC_VASNPRINTF
  gl_FUNC_VSNPRINTF
  gl_STDIO_MODULE_INDICATOR([vsnprintf])
  gl_WCHAR_H
  gl_WCTYPE_H
  gl_XSIZE
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ],
  [gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS],
  [AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])])

# Like AC_LIBSOURCES, except that it does nothing.
# We rely on EXTRA_lib..._SOURCES instead.
AC_DEFUN([gl_LIBSOURCES],
  [])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/link-warning.h
  lib/alloca.c
  lib/alloca_.h
  lib/asnprintf.c
  lib/config.charset
  lib/float+.h
  lib/float_.h
  lib/fseeko.c
  lib/ftw.c
  lib/ftw_.h
  lib/gai_strerror.c
  lib/getaddrinfo.c
  lib/getaddrinfo.h
  lib/getdelim.c
  lib/getline.c
  lib/getpass.c
  lib/getpass.h
  lib/gettext.h
  lib/gettimeofday.c
  lib/inet_ntop.c
  lib/inet_ntop.h
  lib/localcharset.c
  lib/localcharset.h
  lib/lseek.c
  lib/malloc.c
  lib/malloca.c
  lib/malloca.h
  lib/malloca.valgrind
  lib/memmem.c
  lib/minmax.h
  lib/mktime.c
  lib/netinet_in_.h
  lib/pathmax.h
  lib/poll.c
  lib/poll_.h
  lib/printf-args.c
  lib/printf-args.h
  lib/printf-parse.c
  lib/printf-parse.h
  lib/ref-add.sin
  lib/ref-del.sin
  lib/regcomp.c
  lib/regex.c
  lib/regex.h
  lib/regex_internal.c
  lib/regex_internal.h
  lib/regexec.c
  lib/size_max.h
  lib/snprintf.c
  lib/stdbool_.h
  lib/stdint_.h
  lib/stdio_.h
  lib/strcasecmp.c
  lib/strcasestr.c
  lib/strdup.c
  lib/string_.h
  lib/strncasecmp.c
  lib/strndup.c
  lib/strnlen.c
  lib/strpbrk.c
  lib/strptime.c
  lib/strsep.c
  lib/sys_select_.h
  lib/sys_socket_.h
  lib/sys_time_.h
  lib/time_.h
  lib/time_r.c
  lib/timegm.c
  lib/unistd_.h
  lib/vasnprintf.c
  lib/vasnprintf.h
  lib/vsnprintf.c
  lib/wchar_.h
  lib/wctype_.h
  lib/xsize.h
  m4/alloca.m4
  m4/arpa_inet_h.m4
  m4/codeset.m4
  m4/eealloc.m4
  m4/eoverflow.m4
  m4/extensions.m4
  m4/float_h.m4
  m4/fseeko.m4
  m4/ftw.m4
  m4/getaddrinfo.m4
  m4/getdelim.m4
  m4/getline.m4
  m4/getpass.m4
  m4/gettimeofday.m4
  m4/glibc21.m4
  m4/gnulib-common.m4
  m4/include_next.m4
  m4/inet_ntop.m4
  m4/intmax_t.m4
  m4/inttypes_h.m4
  m4/localcharset.m4
  m4/longlong.m4
  m4/lseek.m4
  m4/malloca.m4
  m4/memmem.m4
  m4/minmax.m4
  m4/mktime.m4
  m4/netinet_in_h.m4
  m4/onceonly_2_57.m4
  m4/pathmax.m4
  m4/poll.m4
  m4/regex.m4
  m4/size_max.m4
  m4/snprintf.m4
  m4/socklen.m4
  m4/sockpfaf.m4
  m4/ssize_t.m4
  m4/stdbool.m4
  m4/stdint.m4
  m4/stdint_h.m4
  m4/stdio_h.m4
  m4/strcase.m4
  m4/strcasestr.m4
  m4/strdup.m4
  m4/string_h.m4
  m4/strndup.m4
  m4/strnlen.m4
  m4/strpbrk.m4
  m4/strptime.m4
  m4/strsep.m4
  m4/sys_select_h.m4
  m4/sys_socket_h.m4
  m4/sys_time_h.m4
  m4/time_h.m4
  m4/time_r.m4
  m4/timegm.m4
  m4/tm_gmtoff.m4
  m4/ulonglong.m4
  m4/unistd_h.m4
  m4/vasnprintf.m4
  m4/vsnprintf.m4
  m4/wchar.m4
  m4/wchar_t.m4
  m4/wctype.m4
  m4/wint_t.m4
  m4/xsize.m4
])
