
########### next target ###############
add_definitions(${TAGLIB_CFLAGS})

set(kfile_flac_PART_SRCS kfile_flac.cpp )


kde4_add_plugin(kfile_flac ${kfile_flac_PART_SRCS})



target_link_libraries(kfile_flac  ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES})

install(TARGETS kfile_flac  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_flac.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

