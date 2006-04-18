kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_sid_PART_SRCS kfile_sid.cpp )

kde4_automoc(${kfile_sid_PART_SRCS})

kde4_add_plugin(kfile_sid ${kfile_sid_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_sid )

target_link_libraries(kfile_sid  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kfile_sid )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_sid.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for sid file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes) $(taglib_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_sid.h
#
#kde_module_LTLIBRARIES = kfile_sid.la
#
#kfile_sid_la_SOURCES = kfile_sid.cpp
#kfile_sid_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_sid_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_sid.cpp -o $(podir)/kfile_sid.pot
#
#services_DATA = kfile_sid.desktop
#servicesdir = $(kde_servicesdir)
