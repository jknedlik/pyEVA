Project (Tests)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
SET ( EXECUTABLENAME PagmoStarter)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker

SET ( ${EXECUTABLENAME}_SRCS
	pagmo.cpp
)
set(CMAKE_CXX_FLAGS "-O3 -ggdb -std=c++17 -ltbb -lboost_serialization")


ADD_EXECUTABLE(${EXECUTABLENAME}
	${${EXECUTABLENAME}_SRCS}
)
TARGET_INCLUDE_DIRECTORIES(${EXECUTABLENAME} PRIVATE ${GENEVA_INCLUDE_DIR} ${BOOST_INCLUDE_DIRS})

TARGET_LINK_LIBRARIES (
    ${EXECUTABLENAME}
    ${GENEVA_LIBRARIES}
    ${Boost_LIBRARIES}
    Threads::Threads
    pagmo
)

INSTALL ( TARGETS ${EXECUTABLENAME} DESTINATION ${EXECUTABLENAME} )
