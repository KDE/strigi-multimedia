## Makefile.am for kfileplug

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_mp3.h mp3tech.h mp3info.h

lib_LTLIBRARIES = kfile_mp3.la

kfile_mp3_la_SOURCES = kfile_mp3.cpp mp3tech.c
kfile_mp3_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_mp3_la_LIBADD = $(LIB_KSYCOCA)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kmyapp.pot

services_DATA = kfile_mp3.desktop
servicesdir = $(kde_servicesdir)
