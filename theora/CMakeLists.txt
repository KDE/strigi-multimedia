



########### next target ###############

set(kfile_theora_PART_SRCS kfile_theora.cpp )


kde4_add_plugin(kfile_theora ${kfile_theora_PART_SRCS})



target_link_libraries(kfile_theora  ${KDE4_KIO_LIBS} ${THEORA_LIBRARY} )

install(TARGETS kfile_theora  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_theora.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

