CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(PROJECTNAME Tutorial)

PROJECT(${PROJECTNAME})

FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE(YARP)
FIND_PACKAGE(ICUB)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${OPENCV_INCLUDE_DIR})
INCLUDE(${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
INCLUDE_DIRECTORIES(${ICUB_INCLUDE_DIRS}
                    ${YARP_INCLUDE_DIRS})



LINK_DIRECTORIES(${OPENCV_LINK_DIRECTORIES})
LINK_LIBRARIES(${OPENCV_LIBS})
SET(CMAKE_MODULE_PATH ${YARP_DIR}/conf ${CMAKE_MODULE_PATH})
SET(CMAKE_MODULE_PATH ${ICUB_DIR}/conf ${CMAKE_MODULE_PATH})

					
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ICUB_LINK_FLAGS}")					

# Create everything needed to build our executable.
ADD_EXECUTABLE(findLocationSURF findLocationSURF.cpp SURF.cpp)

TARGET_LINK_LIBRARIES(findLocationSURF ${OpenCV_LIBS})

TARGET_LINK_LIBRARIES(findLocationSURF ${YARP_LIBRARIES})



