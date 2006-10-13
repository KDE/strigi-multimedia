



########### next target ###############

set(kfile_sid_PART_SRCS kfile_sid.cpp )

kde4_automoc(kfile_sid ${kfile_sid_PART_SRCS})

kde4_add_plugin(kfile_sid ${kfile_sid_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_sid )

target_link_libraries(kfile_sid  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_sid  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_sid.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

