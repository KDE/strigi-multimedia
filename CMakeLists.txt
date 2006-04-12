kde4_header()
include(CheckIncludeFileCXX)

add_subdirectory( m3u ) 
add_subdirectory( au ) 
add_subdirectory( avi ) 
add_subdirectory( mpeg ) 
add_subdirectory( wav ) 
add_subdirectory( sid ) 

if(TAGLIB_FOUND)
	check_include_file_cxx("taglib/mpcfile.h" HAVE_TAGLIB_MPCFILE_H)
	MESSAGE(STATUS "HAVE_TAGLIB_MPCFILE_H <${HAVE_TAGLIB_MPCFILE_H}>")
	if(HAVE_TAGLIB_MPCFILE_H)
		add_subdirectory(mpc)
	endif(HAVE_TAGLIB_MPCFILE_H)
	add_subdirectory(flac)
endif(TAGLIB_FOUND)

if(THEORA_FOUND)
	add_subdirectory(theora)
endif(THEORA_FOUND)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KFILE_MP3_SUBDIR)")
message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KFILE_OGG_SUBDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#if include_ogg_SUBDIR
#KFILE_OGG_SUBDIR=ogg
#endif
#
#if include_flac_SUBDIR
#KFILE_FLAC_SUBDIR=flac
#endif
#
#if include_mp3_SUBDIR
#KFILE_MP3_SUBDIR=mp3
#endif
#
#if include_mpc_SUBDIR
#KFILE_MPC_SUBDIR=mpc
#endif
#
#if include_theora_SUBDIR
#KFILE_THEORA_SUBDIR=theora
#endif
#
#SUBDIRS=m3u $(KFILE_MP3_SUBDIR) $(KFILE_MPC_SUBDIR) au avi mpeg wav sid $(KFILE_OGG_SUBDIR) \
#        $(KFILE_FLAC_SUBDIR) $(KFILE_THEORA_SUBDIR)
