kde4_header()




########### next target ###############

set(kfile_au_PART_SRCS kfile_au.cpp )

kde4_automoc(${kfile_au_PART_SRCS})

kde4_add_plugin(kfile_au ${kfile_au_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_au )

target_link_libraries(kfile_au  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kfile_au )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_au.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for au file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_au.h
#
#kde_module_LTLIBRARIES = kfile_au.la
#
#kfile_au_la_SOURCES = kfile_au.cpp 
#kfile_au_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_au_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: 
#	$(XGETTEXT) kfile_au.cpp -o $(podir)/kfile_au.pot
#
#services_DATA = kfile_au.desktop
#servicesdir = $(kde_servicesdir)
