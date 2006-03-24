kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_theora_PART_SRCS kfile_theora.cpp )

kde4_automoc(${kfile_theora_PART_SRCS})

kde4_add_plugin(kfile_theora ${kfile_theora_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_theora )

target_link_libraries(kfile_theora  ${KDE4_KIO_LIBS} ogg vorbis theora )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_theora )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_theora.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for ogg/vorbis file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
## these are the headers for your project
#noinst_HEADERS   = kfile_theora.h
#
#kde_module_LTLIBRARIES = kfile_theora.la
#
#kfile_theora_la_SOURCES = kfile_theora.cpp
#kfile_theora_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_theora_la_LIBADD = $(LIB_KIO) -logg -lvorbis -ltheora
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_theora.cpp -o $(podir)/kfile_theora.pot
#
#services_DATA = kfile_theora.desktop
#servicesdir = $(kde_servicesdir)
