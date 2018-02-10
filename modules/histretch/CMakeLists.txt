# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
project(histretch)

# Find OpenCV, you may need to set OpenCV_DIR variable
# to the absolute path to the directory containing OpenCVConfig.cmake file
# via the command line or GUI
find_package(OpenCV REQUIRED
  NO_MODULE
  PATHS /usr/local
  NO_DEFAULT_PATH)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package(CUDA)

if(CUDA_FOUND)
    INCLUDE(FindCUDA)
  # If the package has been found, several variables will
  # be set, you can find the full list with descriptions
  # in the CUDAConfig.cmake file.
  # Print some message showing some of them
  message(STATUS "CUDA library status:")
  message(STATUS "    toolkit root dir: ${CUDA_TOOLKIT_ROOT_DIR}")
  message(STATUS "    version: ${CUDA_VERSION}")
  message(STATUS "    libraries: ${CUDA_LIBRARIES}")
  message(STATUS "    include path: ${CUDA_INCLUDE_DIRS}")
endif(CUDA_FOUND)

if(CMAKE_VERSION VERSION_LESS "2.8.11")
  # Add OpenCV headers location to your include paths
  include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Declare the executable target built from your sources
# If detect CUDA, then select GPU implementation as prefered method
if(CUDA_FOUND)
  set(FOUND_CUDA 1)
  add_definitions(-D USE_GPU)
  message(STATUS "Configuring for GPU version.")
  file(GLOB histretch-files
    "src/histretch.cpp"
    "../common/*.cpp"	# to be changed with new directory structure
    "../common/*.h"
    "../common/*.hxx"
  ) 
  add_executable(histretch ${histretch-files})
  target_compile_options(histretch PUBLIC -std=c++11)
  # Link your application with OpenCV libraries
target_link_libraries(histretch ${OpenCV_LIBS} ${CUDA_LIBRARIES})
else()
  set(FOUND_CUDA 0)
  message(STATUS "Configuring for non-GPU version.")
  message(STATUS "	Expect a slower speed...")
  file(GLOB histretch-files
    "src/*.h"
    "src/*.hxx"
    "src/histretch.cpp" 
    "../common/*.cpp"	# to be changed with new directory structure
    "../common/*.h"
    "../common/*.hxx"
  ) 
  add_executable(histretch ${histretch-files})
  target_compile_options(histretch PUBLIC -std=c++11)
  # Link your application with OpenCV libraries
  target_link_libraries(histretch ${OpenCV_LIBS})
endif(CUDA_FOUND)