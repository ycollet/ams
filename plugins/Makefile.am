include $(top_srcdir)/common.am

#this is common to all plugins
noinst_LTLIBRARIES = plugin.la

lib_LTLIBRARIES = testplugin.la

plugin_la_SOURCES = plugin.h plugin.cpp
plugin_la_CXXFLAGS = $(CXXFLAGS) -rdynamic
plugin_la_LDFLAGS = -module

testplugin_la_SOURCES = testplugin.h testplugin.cpp moc_testplugin.cpp
testplugin_la_CXXFLAGS = $(QT_CFLAGS) $(CXXFLAGS) -rdynamic
testplugin_la_LIBADD = plugin.la 
testplugin_la_LDFLAGS = -shared -module $(QT_LIBS)
testplugin_la_DEPENDENCIES = $(top_srcdir)/interfaces/subject.h $(top_srcdir)/interfaces/observer.h plugin.la
