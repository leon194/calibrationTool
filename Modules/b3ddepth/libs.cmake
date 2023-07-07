# B3D_DEPTH_DIR should have be defined by main CMakeLists.txt
if (NOT DEFINED B3D_DEPTH_DIR)
    message(WARNING "B3D_DEPTH_DIR is not defined before including b3ddepth/libs.cmake")
    set(B3D_DEPTH_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Expose include directories
set(CORE_INC_DIRS  "${B3D_DEPTH_DIR}/core/include")
set(UTIL_INC_DIRS  "${B3D_DEPTH_DIR}/utils/include")
# "b3depth" module, public headers should follow the same convention as external module
# For external modules to include (e.g. <b3ddepth/core/xxx.h)
set(B3D_DEPTH_INC_DIRS ${CORE_INC_DIRS} ${UTIL_INC_DIRS})

# include B3D_DEPTH_DIR so that to address conflicts when including B3DI at the same time
# set(CORE_EXT_INC ${B3D_DEPTH_DIR}/core/ext)

# Expose b3ddepth library
set(B3D_DEPTH_LIBS b3ddepth)

if(ANDROID)
    if(${ANDROID_ABI} STREQUAL "armeabi-v7a")
        include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
    elseif(${ANDROID_ABI} STREQUAL "arm64-v8a")
        include_directories(${ANDROID_SYSROOT}/usr/include/aarch64-linux-android)
    else()
        include_directories(${ANDROID_SYSROOT}/usr/include/arm-linux-androideabi)
endif()
endif()


# Expose b3ddepth library directories
# Platform dependent settings
if(ANDROID)
    set(B3D_DEPTH_LIB_DIRS ${B3D_DEPTH_DIR}/dsp/jniLibs/${ANDROID_ABI} ${B3D_DEPTH_DIR}/jniLibs/${ANDROID_ABI} )
endif()

if(WIN32 OR (UNIX AND NOT ANDROID))
    set(B3D_DEPTH_LIB_DIRS ${B3D_DEPTH_DIR}/libs)
endif()
