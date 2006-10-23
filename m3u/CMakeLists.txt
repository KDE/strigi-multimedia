



########### next target ###############

set(kfile_m3u_PART_SRCS kfile_m3u.cpp )

kde4_automoc(${kfile_m3u_PART_SRCS})

kde4_add_plugin(kfile_m3u ${kfile_m3u_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_m3u )

target_link_libraries(kfile_m3u  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_m3u  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_m3u.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


