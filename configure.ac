#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([gnucap-geda], [0.0.3-rc5], [felix@salfelder.org])
# AC_CONFIG_SRCDIR([lang_geda.cc])
AC_CONFIG_HEADERS([config.h])

# bug. hardwired.
if test "x${prefix}" = "xNONE"; then
	_prefix="$ac_default_prefix"
else
	_prefix="$prefix"
fi
pkgincludedir="$_prefix/include/$PACKAGE_NAME"

AH_TEMPLATE(GNUCAP_GEDA_PKGINCLUDEDIR, [pkgincludedir])
AC_DEFINE_UNQUOTED([GNUCAP_GEDA_PKGINCLUDEDIR],["$pkgincludedir"])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign silent-rules])
LT_INIT()

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG(C++)

AS_IF([ test x$program_suffix = xNONE ],,[_program_suffix=$program_suffix])
# Checks for libraries.

# Checks for header files.
GC_CPPFLAGS+=$(gnucap-conf$_program_suffix --cppflags)
OLD_CPPFLAGS="$CPPFLAGS"
CPPFLAGS+=" $GC_CPPFLAGS"
AC_CHECK_HEADER([l_dispatcher.h],,[AC_MSG_ERROR([cannot find gnucap headers])])
CPPFLAGS="$OLD_CPPFLAGS"
AC_SUBST([GC_CPPFLAGS])

# Checks for typedefs, structures, and compiler characteristics.

# FIXME: check for gnucap-conf availability
if test "x${prefix}" = "xNONE"; then
gnucap_libdir="$(gnucap-conf$_program_suffix --libdir)"
else
gnucap_libdir="$libdir"/$PACKAGE_NAME
fi
AC_SUBST([gnucap_libdir])

# Checks for library functions.

PKG_CHECK_MODULES(GEDA, [libgeda >= 20100214],
		[
			AC_CHECK_LIB(
				[geda], [s_path_sys_data],
				[
				 geda=yes
				 LIBS+=-lgeda
				],
				[ AC_MSG_ERROR([gEDA package found but header broken]) ],
				[-lgeda])
		],[
				AC_MSG_ERROR([gEDA package not found])
		] )

have_boost_assign=no
AC_CHECK_HEADERS([boost/assign.hpp],
                 [have_boost_assign=yes
                  AC_DEFINE([HAVE_BOOST_ASSIGN], [1], [have boost assign])
                  AC_MSG_NOTICE(Using boost::assign.)
                 ],
                 [
                  AC_MSG_NOTICE(Not using boost::assign.)
                 ])

old_CPPFLAGS=${CPPFLAGS}
CPPFLAGS+=${GEDA_CFLAGS}
AC_CHECK_HEADERS([libgeda/libgeda.h],
                 [ AC_DEFINE([HAVE_GEDA], [1], [geda present]) ],
					  AC_MSG_ERROR([libgeda.h not available]), )
CPPFLAGS=${old_CPPFLAGS}
CXXFLAGS+=" -Wall -pedantic"

gnucap=$(which gnucap)
AC_SUBST([gnucap])

AC_CONFIG_FILES([
	Makefile
	tests/Makefile
	tests/gafrc
	src/Makefile
	include/Makefile
])

AC_CONFIG_FILES([gnucap-geda], [chmod +x gnucap-geda])

AC_OUTPUT

AC_MSG_RESULT([
** Configuration summary for $PACKAGE $VERSION:

   gnucap:       $gnucap
   geda:         $geda
   prefix:       $prefix
   GC_CPPFLAGS:  $GC_CPPFLAGS
   GEDA_CFLAGS:  $GEDA_CFLAGS
   plugindir:    $gnucap_libdir
])
