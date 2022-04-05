
find_path(OPRNAL_INCLUDE_DIR NAMES AL/al.h OpenAL/al.h)
if(OPRNAL_INCLUDE_DIR)
	nb_define(NB_CFG_OPENAL)
	nb_include(${OPENAL_INCLUDE_DIR})
endif()

#include <pulse/simple.h>

find_path(PULSE_INCLUDE_DIR NAMES pulse/simple.h)
if(PULSE_INCLUDE_DIR)
	nb_define(NB_CFG_PULSE)
	nb_include(${PULSE_INCLUDE_DIR})
endif()

if(WIN32)
	find_path(DSOUND_INCLUDE_DIR NAMES dsound.h)
	if(DSOUND_INCLUDE_DIR)
		nb_define(NB_CFG_DSOUND)
		nb_include(${DSOUND_INCLUDE_DIR})
	endif()
else()
	find_path(ALSA_INCLUDE_DIR NAMES alsa/asoundlib.h)
	if(ALSA_INCLUDE_DIR)
		nb_define(NB_CFG_ALSA)
		nb_include(${ALSA_INCLUDE_DIR})
	endif()
endif()

nb_add_source_dir(.)