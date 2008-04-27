SET(SRC font.cxx timer.cxx showfps.cxx LibraryLoader.cxx GLObjectList.cxx)

ADD_LIBRARY(showfps SHARED ${SRC})
TARGET_LINK_LIBRARIES(showfps ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARY})

install(TARGETS showfps
        RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
