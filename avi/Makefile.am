## Makefile.am for avi file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_avi.h

kde_module_LTLIBRARIES = kfile_avi.la

kfile_avi_la_SOURCES = kfile_avi.cpp 
kfile_avi_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_avi_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_avi.cpp -o $(podir)/kfile_avi.pot

services_DATA = kfile_avi.desktop
servicesdir = $(kde_servicesdir)
