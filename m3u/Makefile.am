## Makefile.am for m3u file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_m3u.h

kde_module_LTLIBRARIES = kfile_m3u.la

kfile_m3u_la_SOURCES = kfile_m3u.cpp
kfile_m3u_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_m3u_la_LIBADD = $(LIB_KSYCOCA)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_m3u.cpp -o $(podir)/kfile_m3u.pot

services_DATA = kfile_m3u.desktop
servicesdir = $(kde_servicesdir)
