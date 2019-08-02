AC_INIT([openvpn-python-plugin], [1.0.1], [daniel.uvehag@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_PROG_AR
AC_PROG_CC
AC_PROG_INSTALL

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_CHECK_HEADERS([openvpn-plugin.h], [], [AC_MSG_ERROR([openvpn-plugin.h not found])])

# Plugin directory
AC_ARG_VAR([PLUGINDIR], [Path of plug-in directory @<:@default=LIBDIR/openvpn/plugins@:>@])
if test -n "${PLUGINDIR}"; then
	plugindir="${PLUGINDIR}"
else
	plugindir="\${libdir}/openvpn/plugins"
fi
AC_SUBST([plugindir])

PKG_CHECK_MODULES([PYTHON3], [python3 >= 3.5])

AC_ENABLE_SHARED
AC_DISABLE_STATIC
LT_INIT([shared disable-static])

AM_PATH_PYTHON([3.5])

AC_OUTPUT
