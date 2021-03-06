PROJECT(CODE3)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE(  YARP  REQUIRED )
find_package(LibXML++ REQUIRED)
find_package ( IPP REQUIRED)
include_directories(${LibXML++_INCLUDE_DIRS} ${IPP_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibXML++_LIBRARIES} ${IPP_LIBRARIES})







INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${OPENCV_INCLUDE_DIR} ${YARP_INCLUDE_DIR} ${ICUB_INCLUDE_DIR})
INCLUDE_DIRECTORIES (/home/gabriele/Desktop/ccav/CODE3/lib) 

LINK_DIRECTORIES(${OPENCV_LINK_DIRECTORIES})
LINK_LIBRARIES(${OPENCV_LIBS})
LINK_DIRECTORIES(${YARP_LINK_DIRECTORIES})
LINK_LIBRARIES(${YARP_LIBS})
LINK_DIRECTORIES(${CODE3_BINARY_DIR}/lib)





# Declare the target (an executable)
ADD_EXECUTABLE(GaborMatching  gabriele_detectobj.cpp)

TARGET_LINK_LIBRARIES(GaborMatching ${OpenCV_LIBS} ${LIBS} libGaborMatch)


EXECUTE_PROCESS(COMMAND pkg-config --cflags opencv OUTPUT_VARIABLE CMAKE_C_FLAGS)
string(REPLACE "\n" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

execute_process(COMMAND pkg-config --libs opencv OUTPUT_VARIABLE CMAKE_EXE_LINKER_FLAGS)
string(REPLACE "\n" "" CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

set_target_properties(GaborMatching PROPERTIES COMPILE_FLAGS ${CMAKE_C_FLAGS} LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
