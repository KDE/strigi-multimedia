
########### next target ###############
add_definitions(${TAGLIB_CFLAGS})

set(kfile_flac_PART_SRCS kfile_flac.cpp )

kde4_automoc(${kfile_flac_PART_SRCS})

kde4_add_plugin(kfile_flac ${kfile_flac_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_flac )

target_link_libraries(kfile_flac  ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES})

install(TARGETS kfile_flac  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_flac.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




#original Makefile.am contents follow:

### Makefile.am for FLAC file meta info plugin
#
## set the include path for X, qt, KDE and TagLib
#INCLUDES         = $(all_includes) $(taglib_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_flac.h
#
#kde_module_LTLIBRARIES = kfile_flac.la
#
#kfile_flac_la_SOURCES = kfile_flac.cpp
#kfile_flac_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
#kfile_flac_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_flac.cpp -o $(podir)/kfile_flac.pot
#
#services_DATA = kfile_flac.desktop
#servicesdir = $(kde_servicesdir)
