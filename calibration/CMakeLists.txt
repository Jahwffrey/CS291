cmake_minimum_required(VERSION 3.0)
PROJECT(calib)
FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLUT REQUIRED)
SET(MySources calib.cpp)
ADD_EXECUTABLE(calib ${MySources})
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(calib ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
SET_SOURCE_FILES_PROPERTIES(${MySources} PROTERTIES LANGUAGE CXX)
