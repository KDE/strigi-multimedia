include_directories( ${CMAKE_SOURCE_DIR}/libkdemultimedia )

kde4_add_library(m3u MODULE kfile_m3u.cpp )

target_link_libraries(m3u ${STRIGI_STREAMANALYZER_LIBRARY} ${KDE4_KIO_LIBS})

set_target_properties(m3u PROPERTIES PREFIX strigiea_)

install(TARGETS m3u LIBRARY DESTINATION ${LIB_INSTALL_DIR}/strigi)


