## Makefile.am for sid file meta info plugin

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes) $(taglib_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_sid.h

kde_module_LTLIBRARIES = kfile_sid.la

kfile_sid_la_SOURCES = kfile_sid.cpp
kfile_sid_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_sid_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_sid.cpp -o $(podir)/kfile_sid.pot

services_DATA = kfile_sid.desktop
servicesdir = $(kde_servicesdir)
