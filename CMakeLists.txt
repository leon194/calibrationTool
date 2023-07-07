# Compiles the b3d4_client library and application, version 1.0.0.1
PROJECT(b3d4_client)

cmake_minimum_required(VERSION 3.10)

add_definitions(-std=c++11)

# Disable C4251 Warning: needs to have dll-interface to be used by clients of class
string(REPLACE "/w4251" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
# endif()


option(WITH_B3D_CAMERA "Compile B3D4 Client with b3dcamera module" OFF)
option(WITH_INUITIVE   "Compile B3D4 Client with INU driver"       ON)

# DEFINE preprocessor MACROS
# OpenCV_DIR is an input argument from build-script or CMake GUI
FIND_PACKAGE(OpenCV 3.4.1 EXACT REQUIRED PATHS ${OpenCV_DIR})

message(STATUS "Source Dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    OpenCV path: ${OpenCV_DIR}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Configure B3D_INSTALL_DIR Directories
if(ANDROID)
     add_definitions(-frtti -fexceptions)

    if(WITH_B3D_DEBUG)
        # Install to different project library if WITH_B3D_CAMERA is ON
        if (WITH_B3D_CAMERA)
            set(B3D_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_android/b3dcamera/libs")
        else()
            set(B3D_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_android/b3dstereo/libs")
        endif()
    elseif(WITH_CPU)
	    set(B3D_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/CPU")
    else()
        # WITH_B3D_CAMERA is definitely OFF
        # Install to sample_android directory when build release
        set(B3D_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sample_android/application/b3dstereo/libs")
    endif()

elseif(WIN32)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(EXE_DIR  "${CMAKE_BINARY_DIR}/TestAppWin/Debug/")
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(EXE_DIR  "${CMAKE_BINARY_DIR}/TestAppWin/Release/")
    else()
        set(EXE_DIR  "${CMAKE_BINARY_DIR}/TestAppWin/Release/")
    endif()
elseif(UNIX)
    # On Linux, only install when WITH_B3D_DEBUG is OFF
    set(B3D_INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test_unix)
endif()


# Start building TARGETS
## B3DUtils module
add_subdirectory(Modules/b3dutils)
add_subdirectory(Modules/b3d4client)

## TESTAPP windows module
if(WIN32)
    # only build testapp when debugging internally on Windows
    add_subdirectory(TestAppWin)
endif()
