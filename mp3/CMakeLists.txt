



########### next target ###############

ADD_DEFINITIONS(${TAGLIB_CFLAGS})

set(kfile_mp3_PART_SRCS kfile_mp3.cpp )

kde4_automoc(${kfile_mp3_PART_SRCS})

kde4_add_plugin(kfile_mp3 ${kfile_mp3_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_mp3 )

target_link_libraries(kfile_mp3  ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES} )

install(TARGETS kfile_mp3  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_mp3.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

