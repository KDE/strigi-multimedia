## Makefile.am for ogg/vorbis file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_theora.h

kde_module_LTLIBRARIES = kfile_theora.la

kfile_theora_la_SOURCES = kfile_theora.cpp
kfile_theora_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_theora_la_LIBADD = $(LIB_KIO) -logg -lvorbis -ltheora

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_theora.cpp -o $(podir)/kfile_theora.pot

services_DATA = kfile_theora.desktop
servicesdir = $(kde_servicesdir)
