plugindir = @gnucap_libdir@

PLUGIN_LDFLAGS=-module -shared

plugin_LTLIBRARIES = lang_geda.la
lang_geda_la_CPPFLAGS = $(GEDA_CFLAGS) @GC_CPPFLAGS@
lang_geda_la_CXXFLAGS = -std=c++0x
lang_geda_la_LDFLAGS = $(PLUGIN_LDFLAGS)
lang_geda_la_SOURCES = lang_geda.cc d_net.cc d_place.cc \
                       d_port.cc d_rail.cc d_gedasckt.cc

noinst_HEADERS = d_gedasckt.h symbol.h d_place.h d_net.h
