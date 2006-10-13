



########### next target ###############

set(kfile_mpeg_PART_SRCS kfile_mpeg.cpp )

kde4_automoc(kfile_mpeg ${kfile_mpeg_PART_SRCS})

kde4_add_plugin(kfile_mpeg ${kfile_mpeg_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_mpeg )

target_link_libraries(kfile_mpeg  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_mpeg  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_mpeg.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

