## Makefile.am for au file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_au.h

kde_module_LTLIBRARIES = kfile_au.la

kfile_au_la_SOURCES = kfile_au.cpp 
kfile_au_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_au_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: 
	$(XGETTEXT) kfile_au.cpp -o $(podir)/kfile_au.pot

services_DATA = kfile_au.desktop
servicesdir = $(kde_servicesdir)
