SET (libgto_SRCS
	FlexLexer.cpp
	Parser.cpp
	RawData.cpp
	Reader.cpp
	Utilities.cpp
	Writer.cpp
	zhacks.cpp
)

SET (libgto_HEADERS
	EXTProtocols.h
	FlexLexer.h
	Header.h
	Parser.h
	Protocols.h
	RawData.h
	Reader.h
	Utilities.h
	Writer.h
	zhacks.h
	)

include_directories( 
	         ${CMAKE_SOURCE_DIR}/lib/
	         )

# libgto library
add_library( gto ${libgto_HEADERS} ${libgto_SRCS} )

