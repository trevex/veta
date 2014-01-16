AC_INIT([Veta], [0.1], [niklas.voss@gmail.com], [veta], [http://nikvoss.com/])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 no-define foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX

AC_ARG_WITH([v8-include-path],
  [AS_HELP_STRING([--with-v8-include-path],
    [location of the v8 headers, defaults to /usr/include/v8])],
  [V8_CFLAGS="-I$withval"],
  [V8_CFLAGS='-I/usr/include/v8'])
AC_SUBST([V8_CFLAGS])

AC_ARG_WITH([v8-lib-path],
  [AS_HELP_STRING([--with-v8-lib-path], [location of the v8 libraries])],
  [V8_LIBS="-L$withval -lv8"],
  [V8_LIBS='-lv8'])
AC_SUBST([V8_LIBS])

AC_ARG_WITH([libcppa-include-path],
  [AS_HELP_STRING([--with-libcppa-include-path],
    [location of the libcppa headers, defaults to /usr/local/include/])],
  [LIBCPPA_CFLAGS="-I$withval"])
AC_SUBST([LIBCPPA_CFLAGS])

AC_ARG_WITH([libcppa-lib-path],
  [AS_HELP_STRING([--with-libcppa-lib-path], [location of the libcppa libraries])],
  [LIBCPPA_LIBS="-L$withval -lcppa"],
  [LIBCPPA_LIBS='-lcppa'])
AC_SUBST([LIBCPPA_LIBS])

AC_CONFIG_FILES([Makefile])

AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11

AC_OUTPUT

