
SET(BASS_ROOT_PATH F:/dok/kode/libraries/bass24/c )

FIND_PATH( BASS_INCLUDE_DIR NAMES bass.h 
		PATHS ${BASS_ROOT_PATH}
	)

FIND_LIBRARY( BASS_LIB NAMES bass
		PATHS ${BASS_ROOT_PATH}/
	) 


