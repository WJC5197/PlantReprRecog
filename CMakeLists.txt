# cmake version
cmake_minimum_required(VERSION 3.10)

######### QT ##########
project(PRR VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(OpenMP)

set(ORANGI_PI OFF)

if(WIN32)
        set(CMAKE_PREFIX_PATH "D:/Qt/6.5.0/msvc2019_64")
else()
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

message(STATUS "${OpenMP_CXX_FLAGS}")#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets LinguistTools Multimedia MultimediaWidgets Mqtt)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets LinguistTools Multimedia MultimediaWidgets Mqtt)
include_directories({OpenCV_INCLUDE_DIRS})

set(TS_FILES ./src/qt/PRR_zh_CN.ts)

set(PROJECT_SOURCES
        ./src/qt/widget/terminal.h
        ./src/qt/widget/terminal.cpp

        ./src/qt/main.cpp

        ./src/qt/prr/prrwin.h
        ./src/qt/prr/prrwin.cpp
        ./src/qt/prr/prrwin.ui
        
        ./src/qt/imgset/imagesettings.h
        ./src/qt/imgset/imagesettings.cpp
        ./src/qt/imgset/imagesettings.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PRR
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET PRR APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(PRR SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(PRR
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

# libraries variable
set(LIBS 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Mqtt
    ${OpenCV_LIBS}
)
if(ORANGI_PI)
    set(LIBS 
        ${LIBS}
        wiringPi
        wiringPiDev
        pthread
        m
        crypt
    )
endif()
if(WIN32)
    set(LIBS 
        ${LIBS}
        OpenMP::OpenMP_CXX
	)
endif()

target_link_libraries(PRR PRIVATE ${LIBS})

set_target_properties(PRR PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS PRR
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PRR)
endif()

######### CV ##########
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

aux_source_directory(./test TEST_LIST)

# imgTest
add_executable(ImgTest ./src/cv/imgTest.cpp)
target_link_libraries(ImgTest ${OpenCV_LIBS})

# calibrate 
add_executable(Calib ./src/cv/calibrate.cpp)
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
file(GLOB HEADERS ./src/cv/*.hpp)
source_group("Headers" FILES ${HEADERS})

add_library(Util ${HEADERS})
set_target_properties(Util PROPERTIES LINKER_LANGUAGE CXX)