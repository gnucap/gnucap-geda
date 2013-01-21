#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([gnucap-geda], [0.0.1], [felix@salfelder.org])
# AC_CONFIG_SRCDIR([lang_geda.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
LT_INIT()

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_LANG(C++)


# Checks for libraries.

# Checks for header files.
GC_CPPFLAGS+=$(gnucap-conf --cppflags)
AC_SUBST([GC_CPPFLAGS])

# Checks for typedefs, structures, and compiler characteristics.

# FIXME: check for gnucap-conf availability
gnucap_libdir=$(gnucap-conf --libdir)
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

AC_CONFIG_FILES([Makefile examples/Makefile
                 src/Makefile
                 include/Makefile])

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
