plugindir = @gnucap_libdir@

PLUGIN_LDFLAGS=-module -shared

plugin_LTLIBRARIES = lang_geda.la
lang_geda_la_CPPFLAGS = $(GEDA_CFLAGS)
lang_geda_la_LDFLAGS = $(PLUGIN_LDFLAGS)
lang_geda_la_SOURCES = lang_geda.cc d_net.cc d_place.cc
