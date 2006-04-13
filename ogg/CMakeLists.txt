kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_ogg_PART_SRCS kfile_ogg.cpp vcedit.c )

kde4_automoc(${kfile_ogg_PART_SRCS})

kde4_add_plugin(kfile_ogg ${kfile_ogg_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_ogg )

target_link_libraries(kfile_ogg ${VORBIS_LIBS} ${VORBISFILE_LIBS} ${KDE4_KIO_LIBS} ogg vorbis vorbisfile )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_ogg )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_ogg.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for ogg/vorbis file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_ogg.h
#
#kde_module_LTLIBRARIES = kfile_ogg.la
#
#kfile_ogg_la_SOURCES = kfile_ogg.cpp vcedit.c
#kfile_ogg_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_ogg_la_LIBADD = $(LIB_KIO) -logg -lvorbis -lvorbisfile
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_ogg.cpp -o $(podir)/kfile_ogg.pot
#
#services_DATA = kfile_ogg.desktop
#servicesdir = $(kde_servicesdir)
