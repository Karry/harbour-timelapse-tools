#
# Copyright (c) 2022 Lukas Karas <lukas.karas@centrum.cz>
#
# Structure of OSMScout-Sailfish cmake scripts was inspired by DigiKam project
#
# Redistribution and use is allowed according to the terms of the BSD license.

cmake_policy(SET CMP0048 NEW)
project(harbour-timelapse-tools VERSION 0.1.0 LANGUAGES C CXX)

# project version
set(TIMELAPSE_TOOLS_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})
set(TIMELAPSE_TOOLS_MINOR_VERSION ${PROJECT_VERSION_MINOR})
set(TIMELAPSE_TOOLS_PATCH_VERSION ${PROJECT_VERSION_PATCH})

set(TIMELAPSE_TOOLS_SUFFIX_VERSION "")
set(TIMELAPSE_TOOLS_VERSION_SHORT  "${TIMELAPSE_TOOLS_MAJOR_VERSION}.${TIMELAPSE_TOOLS_MINOR_VERSION}.${TIMELAPSE_TOOLS_PATCH_VERSION}")
set(TIMELAPSE_TOOLS_VERSION_STRING "${TIMELAPSE_TOOLS_VERSION_SHORT}${TIMELAPSE_TOOLS_SUFFIX_VERSION}")


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeMod/Version.h.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/privateinclude/harbour-timelapse-tools/private/Version.h")

set(CMAKE_MIN_VERSION    "3.9.2") # CMake available on Travis CI is 3.9.2
set(QT_MIN_VERSION       "5.6.0") # QList::constBegin was introduced in Qt 5.6

cmake_minimum_required(VERSION ${CMAKE_MIN_VERSION})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_HOME_DIRECTORY}/CMakeMod")

# Local macros
include(MacroUtils)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckPrototypeDefinition)
include(CheckCCompilerFlag)
include(CheckTypeSize)
include(CheckFunctionExists)
include(ExternalProject)

# ==================================================================================================
# Find dependencies

find_package( Qt5 ${QT_MIN_VERSION}
              REQUIRED NO_MODULE COMPONENTS
              Core
              Gui
              LinguistTools
              Multimedia
              Qml
              Quick
              Svg
)
find_package(Qt5LinguistTools)
find_package(SailfishApp) # https://github.com/sailfish-sdk/libsailfishapp

add_subdirectory(dependencies)

# ==================================================================================================
# Compiler flags

# see https://doc.qt.io/qtcreator/creator-debugging-qml.html for more details
option(QT_QML_DEBUG "Build with QML debugger support" OFF)
if (QT_QML_DEBUG)
    add_definitions( -DQT_QML_DEBUG)
else()
    set(QT_QML_DEBUG FALSE)
endif()

option(SANITIZER "Build with sanitizer" none)
if(SANITIZER STREQUAL "address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
elseif(SANITIZER STREQUAL "undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
elseif(SANITIZER STREQUAL "thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
else()
    set(SANITIZER "none")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
if (NOT BUILD_TYPE_LOWER STREQUAL "debug")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(TIMELAPSE_ENABLE_IPO TRUE)
    else()
        message(WARNING "IPO is not supported")
        set(TIMELAPSE_ENABLE_IPO FALSE)
    endif()
else()
    set(TIMELAPSE_ENABLE_IPO FALSE)
endif()

# global settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # build should fail when compiler don't support standard defined by CMAKE_CXX_STANDARD
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions( -Wall -Wextra -pedantic -Wno-unused-function )
add_definitions( -fno-omit-frame-pointer -fstack-protector-all -fPIC )

# Do *NOT* turn assertions off in release and rel-with-deb-info modes.
# CMakes does that by default despite any recent documentation
# recommending against the practice.
string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string( REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
string( REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
string( REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# add function name to Qt log messages (even in release build)
add_definitions( -DQT_MESSAGELOGCONTEXT )

add_definitions(${LIBSAILFISHAPP_DEFINITIONS})

# CMake doesn't add "-pie" by default for executables (CMake issue #14983)
# it is necessary for accelerated start by mapplauncherd,
# see https://git.sailfishos.org/mer-core/mapplauncherd for details
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

set(HARBOUR_APP_NAME harbour-timelapse-tools)

# ==================================================================================================
# main app

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(HEADER_FILES
    src/osmscout/util/CmdLineParsing.h
    src/osmscout/util/String.h
    src/Arguments.h
    src/CameraModel.h
    src/IconProvider.h
    src/QmlCameraDevice.h
    src/QmlTimeLapseCapture.h
    src/TimeLapseTools.h
    )

# keep qml files in source list - it makes qtcreator happy
# find qml -type f
set(QML_FILES
    qml/custom/Utils.js
    qml/pages/Capture.qml
    qml/pages/Cover.qml
    qml/pages/MainPage.qml
    qml/pages/NewTimeLapsePage.qml
    qml/main.qml)

set(SOURCE_FILES
    src/osmscout/util/CmdLineParsing.cpp
    src/osmscout/util/String.cpp
    src/CameraModel.cpp
    src/QmlCameraDevice.cpp
    src/QmlTimeLapseCapture.cpp
    src/TimeLapseTools.cpp)

# XML files with translated phrases.
# You can add new language translation just by adding new entry here, and run build.
# Newly created *.ts file may be edited by linguist tool.
set(TRANSLATION_TS_FILES
    translations/en.ts

    translations/cs.ts
    )

# source files that may contains translatable phrases (see tr, qsTr, QT_TRANSLATE_NOOP functions)
set(TRANSLATION_SOURCE_FILES
    ${SOURCE_FILES}
    ${QML_FILES}
    )

# See http://doc.qt.io/qt-5/cmake-manual.html
#
# qt5_create_translation macro process translations in two steps:
#
#  - collect available phrases inside TRANSLATION_SOURCE_FILES
#    with lupdate tool and put them into *.ts files if some is missing there.
#
#  - create binary translation files (*.qm) from translated phrases available
#    in *.ts files with lrelease tool
qt5_create_translation(TRANSLATION_QM_FILES
    ${TRANSLATION_SOURCE_FILES}
    ${TRANSLATION_TS_FILES})

add_custom_target(translations ALL DEPENDS ${TRANSLATION_QM_FILES})

add_executable(harbour-timelapse-tools ${SOURCE_FILES} ${HEADER_FILES} ${QML_FILES})
set_property(TARGET harbour-timelapse-tools PROPERTY CXX_STANDARD 17)
set_property(TARGET harbour-timelapse-tools PROPERTY INTERPROCEDURAL_OPTIMIZATION ${TIMELAPSE_ENABLE_IPO})
add_dependencies(harbour-timelapse-tools translations)

target_include_directories(harbour-timelapse-tools PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/privateinclude
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LIBSAILFISHAPP_INCLUDE_DIRS}
)

target_link_libraries(harbour-timelapse-tools
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Svg

    ${LIBSAILFISHAPP_LIBRARIES}
    TimeLapse::TimeLapse
)

# https://github.com/sailfish-sdk/cmakesample/blob/master/CMakeLists.txt
install(TARGETS harbour-timelapse-tools
RUNTIME DESTINATION bin)

install(DIRECTORY qml
DESTINATION share/${HARBOUR_APP_NAME})

install(FILES ${TRANSLATION_QM_FILES}
DESTINATION share/${HARBOUR_APP_NAME}/translations)

install(FILES
    ${DEP_LIBS}
    DESTINATION share/${HARBOUR_APP_NAME}/lib)

install(FILES
    ${ffmpeg_TAR}
    DESTINATION share/${HARBOUR_APP_NAME}/bin)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dependencies/installroot/etc/ImageMagick-6
    DESTINATION share/${HARBOUR_APP_NAME}/etc)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dependencies/installroot/lib/ImageMagick-6.9.12
    DESTINATION share/${HARBOUR_APP_NAME}/lib
    FILES_MATCHING PATTERN "*.so" PATTERN "*.xml")

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dependencies/installroot/share/ImageMagick-6
    DESTINATION share/${HARBOUR_APP_NAME}/share)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dependencies/installroot/lib/libgphoto2_port
    DESTINATION share/${HARBOUR_APP_NAME}/lib
    FILES_MATCHING PATTERN "*.so")

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dependencies/installroot/lib/libgphoto2/2.5.29
    DESTINATION share/${HARBOUR_APP_NAME}/lib/libgphoto2
    FILES_MATCHING PATTERN "*.so")

install(FILES harbour-timelapse-tools.desktop
DESTINATION share/applications)

install(FILES icons/86x86/harbour-timelapse-tools.png
DESTINATION share/icons/hicolor/86x86/apps)

install(FILES icons/108x108/harbour-timelapse-tools.png
DESTINATION share/icons/hicolor/108x108/apps)

install(FILES icons/128x128/harbour-timelapse-tools.png
DESTINATION share/icons/hicolor/128x128/apps)

install(FILES icons/256x256/harbour-timelapse-tools.png
DESTINATION share/icons/hicolor/256x256/apps)

install(FILES icons/172x172/harbour-timelapse-tools.png
DESTINATION share/icons/hicolor/172x172/apps)

# ==================================================================================================
# Print build configuration

## print all variables
#message(STATUS "contractionhierarchies: "contractionhierarchies)
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

message(STATUS "")
message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "SFOS TimeLapse tools ${TIMELAPSE_TOOLS_VERSION_STRING} build configuration results")

message(STATUS "")
message(STATUS "Build options:")

message(STATUS " Build type .............................. ${CMAKE_BUILD_TYPE}")
message(STATUS " Interprocedural optimizations ........... ${TIMELAPSE_ENABLE_IPO}")
message(STATUS " Compiler flags .......................... ${CMAKE_CXX_FLAGS}")
message(STATUS " Linker flags ............................ ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS " Sanitizer ............................... ${SANITIZER}")
message(STATUS " QT_QML_DEBUG ............................ ${QT_QML_DEBUG}")

message(STATUS "")
message(STATUS "Requiered dependencies:")

PRINT_LIBRARY_STATUS("Qt5"                    "http://www.qtsoftware.com"                       "(version >= ${QT_MIN_VERSION})"     Qt5_FOUND)
PRINT_LIBRARY_STATUS("SailfishApp library"     "https://github.com/sailfish-sdk/libsailfishapp" ""      LIBSAILFISHAPP_FOUND)

message(STATUS "")

if(Qt5_FOUND AND
   LIBSAILFISHAPP_FOUND)
    message(STATUS      " TimeLapse tools can be compiled ......... YES")
else()
    message(FATAL_ERROR " TimeLapse tools will be compiled ........ NO  (Look README file for more details about dependencies)")
endif()

message(STATUS "----------------------------------------------------------------------------------")
message(STATUS "")
