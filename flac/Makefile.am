## Makefile.am for FLAC file meta info plugin

# set the include path for X, qt, KDE and TagLib
INCLUDES         = $(all_includes) $(taglib_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_flac.h

kde_module_LTLIBRARIES = kfile_flac.la

kfile_flac_la_SOURCES = kfile_flac.cpp
kfile_flac_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
kfile_flac_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_flac.cpp -o $(podir)/kfile_flac.pot

services_DATA = kfile_flac.desktop
servicesdir = $(kde_servicesdir)
