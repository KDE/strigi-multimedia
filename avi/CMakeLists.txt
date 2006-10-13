



########### next target ###############

set(kfile_avi_PART_SRCS kfile_avi.cpp )

kde4_automoc(kfile_avi ${kfile_avi_PART_SRCS})

kde4_add_plugin(kfile_avi ${kfile_avi_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_avi )

target_link_libraries(kfile_avi  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_avi  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_avi.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

