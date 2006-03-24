kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kfile_avi_PART_SRCS kfile_avi.cpp )

kde4_automoc(${kfile_avi_PART_SRCS})

kde4_add_plugin(kfile_avi ${kfile_avi_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_avi )

target_link_libraries(kfile_avi  ${KDE4_KIO_LIBS} )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_avi )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_avi.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for avi file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
#KDE_CXXFLAGS = -UQT3_SUPPORT
## these are the headers for your project
#noinst_HEADERS   = kfile_avi.h
#
#kde_module_LTLIBRARIES = kfile_avi.la
#
#kfile_avi_la_SOURCES = kfile_avi.cpp 
#kfile_avi_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_avi_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_avi.cpp -o $(podir)/kfile_avi.pot
#
#services_DATA = kfile_avi.desktop
#servicesdir = $(kde_servicesdir)
