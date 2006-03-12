kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_mp3_PART_SRCS kfile_mp3.cpp )

kde4_automoc(${kfile_mp3_PART_SRCS})

kde4_add_plugin(kfile_mp3 ${kfile_mp3_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_mp3 )

target_link_libraries(kfile_mp3  ${QT_AND_KDECORE_LIBS} kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_mp3 )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_mp3.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for mp3 file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes) $(taglib_includes)
#
## these are the headers for your project
#noinst_HEADERS   = kfile_mp3.h
#
#kde_module_LTLIBRARIES = kfile_mp3.la
#
#kfile_mp3_la_SOURCES = kfile_mp3.cpp
#kfile_mp3_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
#kfile_mp3_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_mp3.cpp -o $(podir)/kfile_mp3.pot
#
#services_DATA = kfile_mp3.desktop
#servicesdir = $(kde_servicesdir)
