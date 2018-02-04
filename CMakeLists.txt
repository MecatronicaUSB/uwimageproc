cmake_minimum_required(VERSION 3.8)
project(uwimageproc)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        modules/histretch/histretch.cpp
        modules/sample/template.cpp
        modules/utils/preprocessing.cpp
        modules/utils/preprocessing.h
        modules/videostrip/build/videostrip.cpp
        modules/videostrip/videostrip.cpp)

add_executable(uwimageproc ${SOURCE_FILES})