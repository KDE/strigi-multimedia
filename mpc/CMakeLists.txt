kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############
ADD_DEFINITIONS(${TAGLIB_INCLUDE_DIR})
set(kfile_mpc_PART_SRCS kfile_mpc.cpp )

kde4_automoc(${kfile_mpc_PART_SRCS})

kde4_add_plugin(kfile_mpc ${kfile_mpc_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_mpc )

target_link_libraries(kfile_mpc  ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES} )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_mpc )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_mpc.desktop )

kde4_footer()



#original Makefile.am contents follow:

### Makefile.am for MPC file meta info plugin
#
## set the include path for X, qt, KDE and TagLib
#INCLUDES         = $(all_includes) $(taglib_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_mpc.h
#
#kde_module_LTLIBRARIES = kfile_mpc.la
#
#kfile_mpc_la_SOURCES = kfile_mpc.cpp
#kfile_mpc_la_LDFLAGS = $(all_libraries) $(taglib_libs) -module $(KDE_PLUGIN)
#kfile_mpc_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_mpc.cpp -o $(podir)/kfile_mpc.pot
#
#services_DATA = kfile_mpc.desktop
#servicesdir = $(kde_servicesdir)
