



########### next target ###############

set(kfile_wav_PART_SRCS kfile_wav.cpp )


kde4_add_plugin(kfile_wav ${kfile_wav_PART_SRCS})



target_link_libraries(kfile_wav  ${KDE4_KIO_LIBS} )

install(TARGETS kfile_wav  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_wav.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

