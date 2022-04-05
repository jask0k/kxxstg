project(kxx)

# suppress console window
if(MINGW AND NOT NB_DEFINE_DEBUG  AND CMAKE_BUILD_TYPE MATCHES Release)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows") 
endif()

file(GLOB_RECURSE KXX_SOURCE_LIST RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cc" "*.c")

add_definitions(-DKXX_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

if(KXX_SOURCE_LIST)
	if(WIN32)
		list(APPEND KXX_SOURCE_LIST main/kxx.rc)
	endif()
	add_executable(kxx ${KXX_SOURCE_LIST})
	target_link_libraries(kxx nbug)
	if(WIN32)
		target_link_libraries(kxx comctl32)
	endif()
else()
	message(SEND_ERROR "[kx] KXX_SOURCE_LIST is empty, nothing to build.")
endif()

