



########### next target ###############

set(kfile_au_PART_SRCS kfile_au.cpp )

kde4_automoc(${kfile_au_PART_SRCS})

kde4_add_plugin(kfile_au ${kfile_au_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_au )

target_link_libraries(kfile_au  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_au  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_au.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

