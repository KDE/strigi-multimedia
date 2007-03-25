include(CheckIncludeFileCXX)

add_subdirectory( m3u )

message (STATUS "port kfile plugins!!!")
if(KFILE_PLUGINS_PORTED)
#add_subdirectory( m3u ) 
add_subdirectory( au ) 
add_subdirectory( avi ) 
add_subdirectory( mpeg ) 
add_subdirectory( wav ) 
add_subdirectory( sid ) 

if(TAGLIB_FOUND)
	check_include_file_cxx("taglib/mpcfile.h" HAVE_TAGLIB_MPCFILE_H)
	if(HAVE_TAGLIB_MPCFILE_H)
		add_subdirectory(mpc)
	endif(HAVE_TAGLIB_MPCFILE_H)
	add_subdirectory(flac)
	add_subdirectory(mp3)
endif(TAGLIB_FOUND)

if(THEORA_FOUND)
	add_subdirectory(theora)
endif(THEORA_FOUND)

if (OGGVORBIS_FOUND)
	add_subdirectory(ogg)
endif(OGGVORBIS_FOUND)

endif(KFILE_PLUGINS_PORTED)
