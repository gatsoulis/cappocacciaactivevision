CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )
FIND_PACKAGE(  YARP  REQUIRED )
find_package(LibXML++ REQUIRED)
find_package(IPP REQUIRED)

#funziona anche senza questo
set(LIBS ${LIBS} ${LibXML++_LIBRARIES} ${IPP_LIBRARIES})


INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR} ${OPENCV_INCLUDE_DIR} ${YARP_INCLUDE_DIR} ${LibXML++_INCLUDE_DIRS} ${IPP_INCLUDE_DIRS})
INCLUDE(${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
LINK_DIRECTORIES(${OPENCV_LINK_DIRECTORIES})
LINK_LIBRARIES(${OPENCV_LIBS})
LINK_DIRECTORIES(${YARP_LINK_DIRECTORIES})
LINK_LIBRARIES(${YARP_LIBS})

#Funziona anche senza questo
EXECUTE_PROCESS(COMMAND pkg-config --cflags opencv OUTPUT_VARIABLE CMAKE_C_FLAGS)
string(REPLACE "\n" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

execute_process(COMMAND pkg-config --libs opencv OUTPUT_VARIABLE CMAKE_EXE_LINKER_FLAGS)
string(REPLACE "\n" "" CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

EXECUTE_PROCESS(COMMAND pkg-config --cflags libxml++-2.6 OUTPUT_VARIABLE CMAKE_C_FLAGS)
string(REPLACE "\n" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
message("CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

execute_process(COMMAND pkg-config --libs libxml++-2.6 OUTPUT_VARIABLE CMAKE_EXE_LINKER_FLAGS)
string(REPLACE "\n" "" CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
message("CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")

set(CMAKE_C_FLAGS "${GNU_FLAGS} -std=gnu99 ")

# The extension is already found. Any number of sources could be listed here. 
add_library (libGaborMatch CSCData.cpp CSCDescriptors.cpp CSCEllipticRegions.cpp CSCFeature.cpp CSCGaborJet.cpp CSCMatrix2D.cpp CSCVector.cpp CSCViewData.cpp camera.cpp convertcvimagegray.cpp convertflt.cpp csctype.cpp cvimage.cpp detectobject.cpp displaycvimage.cpp drawcross.cpp gabordescriptor.cpp gaborfilter.cpp harrisdetector.cpp loadcvimage.cpp module.cpp resize.cpp serializer.cpp cuetemplate.cpp cue.cpp cueprediction.cpp cuemotion.cpp cuecolorkernel.cpp cropresize.cpp drawvector.cpp cueintegration.cpp convert32FTo8U.cpp manageobjlist.cpp cscsave.cpp cscload.cpp saxparsertrack.cpp vergence.cpp cueintegrationR.cpp cuecontrastkernel.cpp drawrect.cpp dogdetector.cpp imgfeatures.cpp utils.cpp sift.cpp indobjtracker.cpp box.cpp drawdisparity.cpp attention.cpp BRIEF.cpp sendinput.cpp attention2.cpp moveneck.cpp saliency.cpp env_alloc.c env_channel.c env_c_math_ops.c env_image.c env_image_ops.c envision.c envision_lib.c env_job_server.c env_log.c env_math.c env_motion_channel.c env_mt_channel.c env_mt_visual_cortex.c env_params.c env_pthread_interface.c env_pyr.c env_stdio_interface.c env_visual_cortex.c)
