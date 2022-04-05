# OpenGL libraries
if(WIN32)
	find_package(OpenGL)
else()
	find_package(OpenGL REQUIRED)
endif()

if(OPENGL_LIBRARIES)
	nb_define(E_CFG_OPENGL)
	if(OPENGL_INCLUDE_DIR)
		nb_include(${OPENGL_INCLUDE_DIR})
	endif(OPENGL_INCLUDE_DIR)
	nb_link( ${OPENGL_LIBRARIES})
endif(OPENGL_LIBRARIES)

# TODO: Direct3D headers
if(WIN32)
	nb_define(E_CFG_DIRECT3D)
endif(WIN32)

if(WIN32)
	find_library(WINMM_LIBRARY NAMES winmm)
	nb_link( ${WINMM_LIBRARY})
else()
	# X11
	find_package(X11 REQUIRED)
	if(X11_FOUND)
		nb_include(${X11_INCLUDE_DIR})
		nb_link(${X11_LIBRARIES}) 		
	endif()

	# xft
	if(X11_Xft_FOUND)
		# xft require freetype2
		find_path(FREETYPE2_INCLUDE_DIR NAMES freetype/config/ftheader.h PATH_SUFFIXES freetype2)
		if(FREETYPE2_INCLUDE_DIR)
			nb_include(${FREETYPE2_INCLUDE_DIR})			
		endif()
		mark_as_advanced(FREETYPE2_INCLUDE_DIR)
		nb_include(${X11_Xft_INCLUDE_PATH})
		nb_link(${X11_Xft_LIB}) 				
	endif()

	# freetype	
	# find_package(Freetype)
	# if(FREETYPE_LIBRARIES)
		# nb_define(E_CFG_FREETYPE)
		#nb_include(${FREETYPE_INCLUDE_DIR_freetype2}) 
		# nb_link( ${FREETYPE_LIBRARIES}) 
	# endif()
endif()

nb_add_source_dir(.)
nb_add_source_dir(font)
