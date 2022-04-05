cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0015 NEW)

macro(nb_dump list_name)
	message(STATUS "[nbug] ${list_name}:")
	if(${list_name})
		foreach(v ${${list_name}})
			message(STATUS "  -> \"${v}\"")
		endforeach()
	else()
		message(STATUS "  -> <empty>")
	endif()
	message(STATUS "")
endmacro()

project(wriggle)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT DEFINED NB_CFG_VERBOSE)
    option(NB_CFG_VERBOSE "Print verbose debug message." OFF)
endif()

if(NB_CFG_VERBOSE)
	add_definitions(-DNB_CFG_VERBOSE)
endif()

if(NOT DEFINED NB_CFG_OPTIMIZE)
    option(NB_CFG_OPTIMIZE "Optimize." ON)
endif()

if(NB_CFG_OPTIMIZE)
	add_definitions(-DNB_CFG_OPTIMIZE)
endif()

if(NOT DEFINED NB_CFG_SRC_LOC)
    option(NB_CFG_SRC_LOC "Show src location in debug message." OFF)
endif()

if(NB_CFG_SRC_LOC)
	add_definitions(-DNB_CFG_SRC_LOC)
endif()


#if(NOT DEFINED UNICODE_BUILD)
#    option(UNICODE_BUILD "Unicode build." ON)
#endif()
#
#if(UNICODE_BUILD)
#   	add_definitions(-DUNICODE)
#endif()

add_definitions(-DUNICODE)

if(NOT DEFINED NB_DEBUG)
    option(NB_DEBUG "Define the NB_DEBUG symbol." OFF)	
endif()

if(NB_DEBUG)
	add_definitions(-DNB_DEBUG)
endif()

#if(CMAKE_BUILD_TYPE MATCHES Release)
#	add_definitions(-DNB_RELEASE)
#endif()

if(WIN32)
	add_definitions(-DNB_WINDOWS)
else()
	add_definitions(-DNB_LINUX)
endif()

if(NOT DEFINED NB_CFG_MEMCHECK)
    option(NB_CFG_MEMCHECK "Memory check." OFF)	
endif()

if(NB_CFG_MEMCHECK)
	add_definitions(-DNB_CFG_DEBUG_MEMORY)
endif()


# option for link type
if(NOT DEFINED BUILD_SHARED_LIBS)
	if(WIN32)
    	option(BUILD_SHARED_LIBS "Build shared library" OFF)
    else()
    	option(BUILD_SHARED_LIBS "Build shared library" ON)
    endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2") 
	
	if(NOT DEFINED ENABLE_GPROF)
	    option(ENABLE_GPROF "GProf." OFF)	
	endif()
	
	if(ENABLE_GPROF)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg") 
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg") 
	endif()
endif()

if(MINGW)
	# mingw rc compiler
	set(CMAKE_RC_COMPILER_INIT windres)
	enable_language(RC)
	set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
	
	#-static-libgcc
	if(NOT BUILD_SHARED_LIBS AND NOT ENABLE_GPROF)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --static-libstdc++ -static-libgcc") 
	endif()
	
	#auto import
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --enable-auto-import")
endif(MINGW)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819 /wd4290 /wd4018 ") 
endif()

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(nbug)
add_subdirectory(z_kxx)
add_subdirectory(z_tools)

