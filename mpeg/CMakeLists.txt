kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_mpeg_PART_SRCS kfile_mpeg.cpp )

kde4_automoc(${kfile_mpeg_PART_SRCS})

kde4_add_plugin(kfile_mpeg ${kfile_mpeg_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_mpeg )

target_link_libraries(kfile_mpeg  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kfile_mpeg )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_mpeg.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for mpeg file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
## these are the headers for your project
#noinst_HEADERS   = kfile_mpeg.h
#
#kde_module_LTLIBRARIES = kfile_mpeg.la
#
#kfile_mpeg_la_SOURCES = kfile_mpeg.cpp
#kfile_mpeg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_mpeg_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_mpeg.cpp -o $(podir)/kfile_mpeg.pot
#
#services_DATA = kfile_mpeg.desktop
#servicesdir = $(kde_servicesdir)
