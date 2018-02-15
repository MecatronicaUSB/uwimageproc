# cmake needs this line
cmake_minimum_required(VERSION 2.8)

# Define project name
project(video_strip_project)

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
# At this level, it should be useful to check minimum required version or exact version
# So far, there are still some issues with OpenCV installations included in ROS
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Holy crap
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

file(GLOB mosaic-include-files
    "../common/*.h"
    "../common/*.hxx"
    "../common/*.cpp"
    "include/*.h"
    "include/*.hpp"
    "src/*.cpp"
)

add_executable(videostrip ${mosaic-include-files})
target_compile_options(videostrip PUBLIC -std=c++11)

 # Declare the executable target built from your sources
if(CUDA_FOUND)
  # Add definition to CUDA SUPPORT
  add_definitions(-D USE_GPU)
  message(STATUS "Configuring for GPU version.")
  # Link your application with OpenCV libraries
 target_link_libraries(videostrip ${OpenCV_LIBS} ${CUDA_LIBRARIES})
else()
  message(STATUS "Configuring for non-GPU version.")
  message(STATUS "	Expect a slower speed...")
  # Link your application with OpenCV libraries
 target_link_libraries(videostrip ${OpenCV_LIBS})
endif(CUDA_FOUND)



