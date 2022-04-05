
if(WIN32)
	find_library(SHLWAPI_LIBRARY NAMES shlwapi)
	nb_link(${SHLWAPI_LIBRARY})
else()
	# pthread
	find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h)
	find_library(PTHREAD_LIBRARIES NAMES pthread)
	if(PTHREAD_LIBRARIES)
		nb_define(E_CFG_THREAD)
		nb_include(${PTHREAD_INCLUDE_DIR})
		nb_link(${PTHREAD_LIBRARIES})
	endif()
	
	# dlopen, dlclose
	find_library(LD_LIBRARIES NAMES dl)
	nb_link(${LD_LIBRARIES})	
	mark_as_advanced(PTHREAD_INCLUDE_DIR PTHREAD_LIBRARIES LD_LIBRARIES)
endif()



nb_add_source_dir(.)
