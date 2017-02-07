plugindir = @gnucap_pkglibdir@
PLUGIN_LDFLAGS=-module -shared
AM_DEFAULT_SOURCE_EXT = .cc

pkginclude_HEADERS = lang_geda.h

plugin_LTLIBRARIES = lang_geda.la f_is.la

lang_geda_la_CPPFLAGS = $(GEDA_CFLAGS) @GC_CPPFLAGS@
lang_geda_la_CXXFLAGS = -std=c++0x
lang_geda_la_LDFLAGS = $(PLUGIN_LDFLAGS)
lang_geda_la_SOURCES = \
	lang_geda.cc d_net.cc d_place.cc \
	d_port.cc d_rail.cc d_gedasckt.cc \
	c_hide.cc \
	c_include.cc signon.cc

f_is_la_CPPFLAGS = $(GEDA_CFLAGS) @GC_CPPFLAGS@
f_is_la_CXXFLAGS = -std=c++0x
f_is_la_LDFLAGS = $(PLUGIN_LDFLAGS)

noinst_HEADERS = d_gedasckt.h symbol.h d_place.h d_net.h
