



########### next target ###############
ADD_DEFINITIONS(${TAGLIB_CFLAGS})
set(kfile_mpc_PART_SRCS kfile_mpc.cpp )


kde4_add_plugin(kfile_mpc ${kfile_mpc_PART_SRCS})



target_link_libraries(kfile_mpc  ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES} )

install(TARGETS kfile_mpc  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kfile_mpc.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

