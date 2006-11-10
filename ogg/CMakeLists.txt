



########### next target ###############

set(kfile_ogg_PART_SRCS kfile_ogg.cpp vcedit.c )

kde4_automoc(${kfile_ogg_PART_SRCS})

kde4_add_plugin(kfile_ogg ${kfile_ogg_PART_SRCS})



target_link_libraries(kfile_ogg ${OGGVORBIS_LIBRARIES} ${KDE4_KIO_LIBS})

install(TARGETS kfile_ogg  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_ogg.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )


