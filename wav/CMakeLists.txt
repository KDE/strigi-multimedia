



########### next target ###############

set(kfile_wav_PART_SRCS kfile_wav.cpp )

kde4_automoc(${kfile_wav_PART_SRCS})

kde4_add_plugin(kfile_wav ${kfile_wav_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_wav )

target_link_libraries(kfile_wav  ${KDE4_KIO_LIBS} )

install_targets(${PLUGIN_INSTALL_DIR} kfile_wav )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_wav.desktop )




#original Makefile.am contents follow:

### Makefile.am for wav file meta info plugin
#
## set the include path for X, qt and KDE
#INCLUDES         = $(all_includes)
## these are the headers for your project
#noinst_HEADERS   = kfile_wav.h
#
#kde_module_LTLIBRARIES = kfile_wav.la
#
#kfile_wav_la_SOURCES = kfile_wav.cpp 
#kfile_wav_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#kfile_wav_la_LIBADD = $(LIB_KIO)
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) kfile_wav.cpp -o $(podir)/kfile_wav.pot
#
#services_DATA = kfile_wav.desktop
#servicesdir = $(kde_servicesdir)
