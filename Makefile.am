## Makefile.am for kfileplug

# set the include path for X, qt and KDE
INCLUDES         = $(all_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_mp3.h mp3tech.h mp3info.h \
                   kfile_ogg.h \
                   kfile_m3u.h

kde_module_LTLIBRARIES = kfile_mp3.la kfile_m3u.la $(KFILE_OGG)

EXTRA_LTLIBRARIES = kfile_ogg.la
# KFILE_OGG expands to kfile_ogg.la when enabled.

kfile_mp3_la_SOURCES = kfile_mp3.cpp mp3tech.c
kfile_mp3_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_mp3_la_LIBADD = $(LIB_KSYCOCA)

kfile_m3u_la_SOURCES = kfile_m3u.cpp
kfile_m3u_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_m3u_la_LIBADD = $(LIB_KSYCOCA)

# yes, it's true, automake sucks
kfile_ogg_la_SOURCES = kfile_ogg.cpp
kfile_ogg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
kfile_ogg_la_LIBADD = $(LIB_KSYCOCA) -logg -lvorbis -lvorbisfile

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_mp3.cpp -o $(podir)/kfile_mp3.pot
	$(XGETTEXT) kfile_m3u.cpp -o $(podir)/kfile_m3u.pot
	$(XGETTEXT) kfile_ogg.cpp -o $(podir)/kfile_ogg.pot

services_DATA = kfile_mp3.desktop kfile_m3u.desktop $(KFILE_OGG_DESKTOP)
servicesdir = $(kde_servicesdir)
