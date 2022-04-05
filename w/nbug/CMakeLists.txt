project(nbug)


unset(NB_DEFINITION_LIST)      # i.e. "UNICODE;_DEBUG;..."
unset(NB_SOURCE_LIST)          # i.e. "core/Thread.cpp;core/aa.cpp;..."
unset(NB_INCLUDE_DIR_LIST)     # i.e. "C:/MingGW/include;D:/maylibs/include;..."
unset(NB_DEPEND_LIST)          # i.e. "C:/MingGW/lib/opengl32.a;D:/maylibs/lib/foo.a;..."
unset(NB_SUB_MODULE_LIST)      # dirs which contain "dir.cmake" file. ie "al;core;..."

set(NB_SRC_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
unset(NB_CURRENT_SUB_DIR)

macro(nb_add_source_dir dir)
	# require NB_SRC_ROOT_DIR and NB_CURRENT_SUB_DIR, absolute path.
	unset(srcs)
	
	if("${dir}" STREQUAL ".")
		aux_source_directory("${NB_CURRENT_SUB_DIR}" srcs) # avoid "foo/./bar.c"
	else()
		aux_source_directory("${NB_CURRENT_SUB_DIR}/${dir}" srcs)
	endif()

	foreach(v ${srcs})
		file(RELATIVE_PATH v "${NB_SRC_ROOT_DIR}" "${v}")
		list(APPEND NB_SOURCE_LIST "${v}")
	endforeach()
endmacro()

macro(nb_define)
	foreach(v ${ARGV})
		list(APPEND NB_DEFINITION_LIST ${v})
	endforeach()
endmacro()

macro(nb_include)
	foreach(v ${ARGV})
		list(APPEND NB_INCLUDE_DIR_LIST ${v})
	endforeach()
endmacro()

macro(nb_link)
	foreach(v ${ARGV})
		list(APPEND NB_DEPEND_LIST ${v})
	endforeach()
endmacro()

# generate module list
file(GLOB_RECURSE NB_DIR_DOT_CMAKE_LIST RELATIVE "${NB_SRC_ROOT_DIR}" "*dir.cmake")
if(NB_DIR_DOT_CMAKE_LIST)
	foreach(subconfig ${NB_DIR_DOT_CMAKE_LIST})
		if("${subconfig}" STREQUAL "dir.cmake")
			set(v ".")
		else()
			string(REGEX REPLACE "(.+)[/\\\\]dir.cmake" "\\1" v "${subconfig}")
		endif()
		#include(${subconfig})
		list(APPEND NB_SUB_MODULE_LIST ${v})
	endforeach()
	list(REMOVE_DUPLICATES NB_SUB_MODULE_LIST)
	list(SORT NB_SUB_MODULE_LIST)
endif()

# configure modules
foreach(v ${NB_SUB_MODULE_LIST})
	set(NB_CURRENT_SUB_DIR "${NB_SRC_ROOT_DIR}/${v}")
	include("${NB_CURRENT_SUB_DIR}/dir.cmake")
endforeach()


# defines
if(NB_DEFINITION_LIST)
	list(REMOVE_DUPLICATES NB_DEFINITION_LIST)
	list(SORT NB_DEFINITION_LIST)
	foreach(v ${NB_DEFINITION_LIST})
		if("${v}" MATCHES "-D.+|/D.+")
			add_definitions("${v}")
		else()
			add_definitions("-D${v}")
		endif()
	endforeach()
endif()

# includes
if(NB_INCLUDE_DIR_LIST)
	list(REMOVE_DUPLICATES NB_INCLUDE_DIR_LIST)
	list(SORT NB_INCLUDE_DIR_LIST)
	include_directories(${NB_INCLUDE_DIR_LIST})
endif()

# target
if(WIN32 AND EXISTS res/nbug.rc)
	list(APPEND NB_SOURCE_LIST res/nbug.rc)
endif()

if(NB_SOURCE_LIST)
	list(REMOVE_DUPLICATES NB_SOURCE_LIST)
	list(SORT NB_SOURCE_LIST)
	add_library(nbug ${NB_SOURCE_LIST})
else()
	message(SEND_ERROR "[nbug] NB_SOURCE_LIST is empty, nothing to build.")
endif()

# depends
if(NB_DEPEND_LIST)
	list(REMOVE_DUPLICATES NB_DEPEND_LIST)
	list(SORT NB_DEPEND_LIST)
	target_link_libraries(nbug ${NB_DEPEND_LIST})
endif()

# dump configure result
nb_dump(NB_SUB_MODULE_LIST)
nb_dump(NB_DEFINITION_LIST)
# nb_dump(NB_SOURCE_LIST)
nb_dump(NB_INCLUDE_DIR_LIST)
nb_dump(NB_DEPEND_LIST)

# clean up
unset(NB_DIR_DOT_CMAKE_LIST)
unset(NB_DEFINITION_LIST)
unset(NB_SOURCE_LIST)
unset(NB_INCLUDE_DIR_LIST)
unset(NB_DEPEND_LIST)
unset(NB_SUB_MODULE_LIST)

install(TARGETS nbug 
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
		ARCHIVE DESTINATION bin)
