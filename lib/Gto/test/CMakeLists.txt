include_directories( . )

SET ( testlibgto_SRCS
		main.cpp )

SOURCE_GROUP( src FILES ${testgto_SRCS} )

include_directories( 
	         ${CMAKE_SOURCE_DIR}/lib/
	         )

set ( LIBS ${LIBS}
	  gto
	  z
	 )

ADD_EXECUTABLE( testlibgto ${testlibgto_SRCS} )
target_link_libraries( testlibgto ${LIBS} )
set_target_properties( testlibgto PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/Gto/test/ )

