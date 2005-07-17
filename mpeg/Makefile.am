## Makefile.am for mpeg file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_mpeg.h

kde_module_LTLIBRARIES = kfile_mpeg.la

kfile_mpeg_la_SOURCES = kfile_mpeg.cpp
kfile_mpeg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_mpeg_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_mpeg.cpp -o $(podir)/kfile_mpeg.pot

services_DATA = kfile_mpeg.desktop
servicesdir = $(kde_servicesdir)
