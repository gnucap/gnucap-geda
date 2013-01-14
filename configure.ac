#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.67])
AC_INIT([gnucap-geda], [0.0.0], [felix@salfelder.org])
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


old_CPPFLAGS=${CPPFLAGS}
CPPFLAGS+=${GEDA_CFLAGS}
AC_CHECK_HEADERS([libgeda/libgeda.h],
                 [ AC_DEFINE([HAVE_GEDA], [1], [geda present]) ],
					  AC_MSG_ERROR([libgeda.h not available]), )
CPPFLAGS=${old_CPPFLAGS}

gnucap=$(which gnucap)
AC_SUBST([gnucap])

AC_CONFIG_FILES([Makefile examples/Makefile
                 src/Makefile
                 include/Makefile
                 gnucap-geda])
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
