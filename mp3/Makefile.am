## Makefile.am for mp3 file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes) $(taglib_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_mp3.h

kde_module_LTLIBRARIES = kfile_mp3.la

kfile_mp3_la_SOURCES = kfile_mp3.cpp
kfile_mp3_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
kfile_mp3_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_mp3.cpp -o $(podir)/kfile_mp3.pot

services_DATA = kfile_mp3.desktop
servicesdir = $(kde_servicesdir)
