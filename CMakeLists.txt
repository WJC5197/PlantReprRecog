# project name
project(PlantReprRecog C CXX)

# using C++20
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++20 ")

# cmake version
cmake_minimum_required(VERSION 3.10)

# find Packages
find_package(OpenCV REQUIRED)

# show the message of OpenCV
message(STATUS "OpenCV library status:")
message(STATUS "    version: 	${OpenCV_VERSION}")
message(STATUS "    headers: 	${OpenCV_INCLUDE_DIRS}")
message(STATUS "    libraries: 	${OpenCV_LIBS}")

# link headers
include_directories({OpenCV_INCLUDE_DIRS})

aux_source_directory(./test TEST_LIST)

# Main
add_executable(Main ./src/main.cpp)
target_link_libraries(Main ${OpenCV_LIBS})

# imgTest
add_executable(ImgTest ./src/imgTest.cpp)
target_link_libraries(ImgTest ${OpenCV_LIBS})

# calibrate 
add_executable(Calib ./src/calibrate.cpp)
target_link_libraries(Calib ${OpenCV_LIBS})

# snippetTest
add_executable(Stest ${TEST_LIST})
target_link_libraries(Stest ${OpenCV_LIBS})

# Test
enable_testing()

add_test(
	name Stest
	command $<TARGET_FILE:snippetTest> #<TARGET_FILE:> 指代mytest这个目标的完整路径。
	)

# Util
file(GLOB HEADERS ./src/*.hpp)
source_group("Headers" FILES ${HEADERS})

add_library(Util ${HEADERS})
set_target_properties(Util PROPERTIES LINKER_LANGUAGE CXX)

