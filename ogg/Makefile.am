## Makefile.am for ogg/vorbis file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_ogg.h

kde_module_LTLIBRARIES = kfile_ogg.la

kfile_ogg_la_SOURCES = kfile_ogg.cpp vcedit.c
kfile_ogg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_ogg_la_LIBADD = $(LIB_KIO) -logg -lvorbis -lvorbisfile

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_ogg.cpp -o $(podir)/kfile_ogg.pot

services_DATA = kfile_ogg.desktop
servicesdir = $(kde_servicesdir)
