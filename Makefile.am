include $(top_srcdir)/common.am

ACLOCAL_AMFLAGS = -I m4

AM_CXXFLAGS = -I$(top_srcdir) \
	-DINSTPREFIX=\"$(prefix)\" $(LADCCA_CFLAGS)

SUBDIRS = m4 plugins interfaces events trivia

bin_PROGRAMS = test_autoload
test_autoload_CXXFLAGS = -rdynamic -export-dynamic \
$(CXXFLAGS) $(CFLAGS) $(QT_CFLAGS)

test_autoload_LDFLAGGS = -export-dynamic \
-rdynmaic $(LDFLAGS) $(QT_LIBS)
test_autoload_SOURCES = test_autoload.cpp
test_autoload_LDADD = plugins/plugin.la
