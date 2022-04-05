# PROJECT(nbug/input)

set(NBINPUT_DEPS "")

if(WIN32)
	# nothing
else()
	# X11
	find_package(X11 REQUIRED)
	if(X11_LIBRARIES)
		nb_include(${X11_INCLUDE_DIR})
		nb_link( ${X11_LIBRARIES}) 		
	endif()
	
	# joystick library
	find_path(JOYSTICK_INCLUDE_DIR linux/joystick.h)
	if(JOYSTICK_INCLUDE_DIR)
		nb_define(E_CFG_JOYSTICK)
	endif()
	mark_as_advanced(JOYSTICK_INCLUDE_DIR)
endif()

nb_add_source_dir(.)
