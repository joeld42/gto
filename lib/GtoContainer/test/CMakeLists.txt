include_directories( . )

SET ( testlibgtoContainer_SRCS
		main.cpp )

SOURCE_GROUP( src FILES ${testgtoContainer_SRCS} )

include_directories( 
	         ${CMAKE_SOURCE_DIR}/lib/
	         )

set ( LIBS ${LIBS}
	  gtoContainer
	  gto
	  z
	 )

ADD_EXECUTABLE( testlibgtoContainer ${testlibgtoContainer_SRCS} )
target_link_libraries( testlibgtoContainer ${LIBS} )
set_target_properties( testlibgtoContainer PROPERTIES
		       		 RUNTIME_OUTPUT_DIRECTORY 
		       		 ${CMAKE_SOURCE_DIR}/lib/GtoContainer/test/ )

