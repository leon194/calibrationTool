# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../Modules")

# # Import "b3d4client" module
# set(B3D4_CLIENT_DIR "${MODULE_DIR}/b3d4client")
# include("${B3D4_CLIENT_DIR}/libs.cmake")
# # Exposes: B3D4_CLIENT_INC_DIRS, B3D4_CLIENT_LIB
#
# include_directories(${B3D4_CLIENT_INC_DIRS})

#### set up opencv library
set(OPENCV_LIB ${MODULE_DIR}/thirdparty/opencv3.4.1/libs/${ANDROID_ABI})
set(OPENCV_INC ${MODULE_DIR}/thirdparty/opencv3.4.1/include)
include_directories(${OPENCV_INC})
link_directories(${OPENCV_LIB})
message(${OPENCV_LIB})

# set(B3D_DEPTH_DIR ${MODULE_DIR}/b3d_depth)
#
# ## INCLUDE "b3ddepth" module
# include(${B3D_DEPTH_DIR}/libs.cmake)
# # Exposes: B3D_DEPTH_INC_DIRS, CORE_EXT_INC, B3D_DEPTH_LIBS, B3D_DEPTH_LIB_DIR
# include_directories(${B3D_DEPTH_INC_DIRS})
# include_directories(${CORE_EXT_INC})


## Build all modules related to "b3d4client"
# Start building TARGETS
## B3DUtils module
add_subdirectory(${MODULE_DIR}/b3dutils b3dutils)
## B3DDepth module
# add_subdirectory(${MODULE_DIR}/b3ddepth b3ddepth)
## B3D4Client module
add_subdirectory(${MODULE_DIR}/b3d4client b3d4client)
# add_subdirectory(${B3D4_CLIENT_DIR})


# Import "b3di" module
set(B3DI_DIR "${MODULE_DIR}/libb3di")
include("${B3DI_DIR}/libs.cmake")
# Exposes: B3DI_INC_DIRS, B3DI_LIB_DIRS, B3DI_LIBS, B3DI_DLLS, B3DI_DLL_DIR

# Import "b3dutils" module
set(B3D_UTILS_DIR "${MODULE_DIR}/b3dutils")
include("${B3D_UTILS_DIR}/libs.cmake")
# Exposes: B3D_UTILS_INC_DIRS, B3D_UTILS_LIB_DIRS, B3D_UTILS_LIBS

# Import "b3ddepth" module
set(B3D_DEPTH_DIR "${MODULE_DIR}/b3ddepth")
include("${B3D_DEPTH_DIR}/libs.cmake")
# Exposes: B3D_DEPTH_INC_DIRS, B3D_DEPTH_LIB_DIRS, B3D_DEPTH_LIBS

# Import "b3d4client" module
set(B3D4_CLIENT_DIR "${MODULE_DIR}/b3d4client")
include("${B3D4_CLIENT_DIR}/b3d4client.cmake")
# Exposes: B3D4_CLIENT_INC_DIRS, B3D4_CLIENT_LIB
# Exposes: B3DI_INC_DIRS, B3DI_LIB_DIRS, B3DI_LIBS

# Add include directories here
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${B3DI_INC_DIRS})
include_directories(${B3D_UTILS_INC_DIRS})
include_directories(${B3D_DEPTH_INC_DIRS})
include_directories(${B3D4_CLIENT_INC_DIRS})

link_directories(${B3D_DEPTH_LIB_DIRS})
link_directories(${B3DI_LIB_DIRS})

add_library(
# Sets the name of the library
    b3d4clientJNI
# Sets the library as a shared library
    SHARED
# Provides a relative path to your source file(s).
    b3d4module.cpp
    b3d4module.h
    jnihelpers.cpp
    jnihelpers.h
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# message(FATAL_ERROR "B3D4_CLIENT_LIB: ${B3D4_CLIENT_LIB}")

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
		b3d4clientJNI
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        b3d4client
        opencv_java3
		android
)
