## Makefile.am for MPC file meta info plugin

# set the include path for X, qt, KDE and TagLib
INCLUDES         = $(all_includes) $(taglib_includes)

# these are the headers for your project
noinst_HEADERS   = kfile_mpc.h

kde_module_LTLIBRARIES = kfile_mpc.la

kfile_mpc_la_SOURCES = kfile_mpc.cpp
kfile_mpc_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
kfile_mpc_la_LIBADD = $(LIB_KIO)

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) kfile_mpc.cpp -o $(podir)/kfile_mpc.pot

services_DATA = kfile_mpc.desktop
servicesdir = $(kde_servicesdir)
