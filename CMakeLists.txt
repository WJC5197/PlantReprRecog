# project name
project(PlantReprRecog C CXX)

# using C++11
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -std=c++20 ")

# cmake version
cmake_minimum_required(VERSION 3.10)

# find OpenCV
find_package(OpenCV REQUIRED)

# show the message of OpenCV
message(STATUS "OpenCV library status:")
message(STATUS "    version: 	${OpenCV_VERSION}")
message(STATUS "    headers: 	${OpenCV_INCLUDE_DIRS}")
message(STATUS "    libraries: 	${OpenCV_LIBS}")

# link headers
include_directories({OpenCV_INCLUDE_DIRS})


# 添加源代码文件到SRC_LIST变量中
# aux_source_directory(./src SRC_LIST)
aux_source_directory(./test TEST_LIST)

# Main
    # 生成可执行文件
add_executable(prr ./src/main.cpp)
target_link_libraries(prr ${OpenCV_LIBS})

# Real Time Test
add_executable(rtt ./src/rtt.cpp)
target_link_libraries(rtt ${OpenCV_LIBS})

# hsv threshold picker
add_executable(tsh ./src/threshold.cpp)
target_link_libraries(tsh ${OpenCV_LIBS})

# test
add_executable(snippetTest ${TEST_LIST})
target_link_libraries(snippetTest ${OpenCV_LIBS})

enable_testing()

add_test(
	name snippetTest
	command $<TARGET_FILE:snippetTest> #<TARGET_FILE:> 指代mytest这个目标的完整路径。
	)
