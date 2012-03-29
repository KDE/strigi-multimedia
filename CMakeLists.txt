set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
find_package(Qt4 REQUIRED)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)

include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

include(CheckIncludeFileCXX)

message (STATUS "port strigi-analyzer !!!")
if(KFILE_PLUGINS_PORTED) 

macro_optional_find_package(Theora)
macro_log_feature(THEORA_FOUND "Theora" "A video codec intended for use within the Ogg's project's Ogg multimedia streaming system" "http://www.theora.org" FALSE "" "Required to build the Theora Strigi Analyzer.")


add_subdirectory( avi ) 
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
