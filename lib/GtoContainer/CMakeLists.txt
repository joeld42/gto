SET (libgtoContainer_SRCS
	Component.cpp
	ObjectVector.cpp
	Property.cpp
	PropertyContainer.cpp
	Reader.cpp
	StdProperties.cpp
	Writer.cpp
)

SET (gtoContainer_HEADERS
	Component.h
	Exception.h
	Foundation.h
	ObjectVector.h
	Property.h
	PropertyContainer.h
	Protocol.h
	Reader.h
	StdProperties.h
	TypedProperty.h
	Utilities.h
	Writer.h
)

include_directories( 
     ${CMAKE_SOURCE_DIR}/lib/
     )

# libgtoContainer library
add_library( gtoContainer ${libgtoContainer_HEADERS} ${libgtoContainer_SRCS} )

