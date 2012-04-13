
SET(BULLET_ROOT_PATH F:/code_lab/bullet-2.79 )


FIND_PATH( BULLET_INCLUDE_DIR NAMES btBulletDynamicsCommon.h
		PATHS ${BULLET_ROOT_PATH}/src
	)

FIND_LIBRARY( BULLET_COLLISION_DEBUG_LIBRARY NAMES BulletCollision_debug 
	PATHS ${BULLET_ROOT_PATH}/lib 
	) 
FIND_LIBRARY( BULLET_DYNAMICS_DEBUG_LIBRARY NAMES BulletDynamics_debug 
	PATHS ${BULLET_ROOT_PATH}/lib
	) 
FIND_LIBRARY( BULLET_LINEARMATH_DEBUG_LIBRARY NAMES LinearMath_debug 
	PATHS ${BULLET_ROOT_PATH}/lib
	) 
	
FIND_LIBRARY( BULLET_COLLISION_RELEASE_LIBRARY NAMES BulletCollision
	PATHS ${BULLET_ROOT_PATH}/lib
	) 
FIND_LIBRARY( BULLET_DYNAMICS_RELEASE_LIBRARY NAMES BulletDynamics
	PATHS ${BULLET_ROOT_PATH}/lib
	) 
FIND_LIBRARY( BULLET_LINEARMATH_RELEASE_LIBRARY NAMES LinearMath 
	PATHS ${BULLET_ROOT_PATH}/lib
	) 