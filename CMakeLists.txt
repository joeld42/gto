CMAKE_MINIMUM_REQUIRED( VERSION 2.6.4)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

project( open-gto )

ADD_SUBDIRECTORY( lib/Gto )
ADD_SUBDIRECTORY( lib/GtoContainer )


# Tests
ADD_SUBDIRECTORY(lib/Gto/test)
ADD_SUBDIRECTORY(lib/GtoContainer/test)

