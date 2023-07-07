# Update the follow version string when new source files are added
# 0.1.0.5

set(B3D4_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MODULES_DIR "${B3D4_DIR}/..")


set(B3D4_CLIENT_INC "${B3D4_DIR}/include/b3d4client")
set(B3D4_CLIENT_INC_DIRS "${B3D4_DIR}/include/" ${B3D4_CLIENT_INC})

set(B3D4_CLIENT_SRC "${B3D4_DIR}/src")

# message(FATAL_ERROR, "B3D4_CLIENT_SRC: ${B3D4_CLIENT_SRC}")
file(GLOB B3D4_HEADER_FILES
    ${B3D4_CLIENT_INC}/*.h
)

file(GLOB B3D4_SRC_FILES
    ${B3D4_CLIENT_SRC}/*.h
    ${B3D4_CLIENT_SRC}/*.cpp
)

#### Configure INCLUDE dependencies
# Import "b3dutils" module
set(B3D_UTILS_DIR "${MODULES_DIR}/b3dutils")
include("${B3D_UTILS_DIR}/libs.cmake")
# Exposes: B3D_UTILS_INC_DIRS, B3D_UTILS_LIB_DIRS, B3D_UTILS_LIBS

## Import "b3di" module:
set(B3DI_DIR "${MODULES_DIR}/libb3di")
include("${B3DI_DIR}/b3di.cmake")
# Exposes: B3DI_INC_DIRS, B3DI_LIB_DIRS, B3DI_LIBS

# Import "b3ddepth" module
set(B3D_DEPTH_DIR "${MODULES_DIR}/b3ddepth")
include("${B3D_DEPTH_DIR}/libs.cmake")
# Exposes: B3D_DEPTH_INC_DIRS, B3D_DEPTH_LIB_DIRS, B3D_DEPTH_LIBS

# message(FATAL_ERROR "B3DI_LIBS: ${B3DI_LIBS}")
if(ANDROID)
    set(B3D4_LINK_LIBS b3dutils b3ddepth ${B3DI_LIBS})
else()
    set(B3D4_LINK_LIBS b3dutils b3ddepth ${B3DI_LIBS} ${OpenCV_LIBS})
endif()

if(WITH_INUITIVE)
    # Include INUDEV module
    set(INUDEV_DIR "${MODULES_DIR}/thirdparty/inudev")
    include("${INUDEV_DIR}/libs.cmake")
    # Exposes: target library "inudev", INUDEV_INC, INUDEV_LIB_DIR, INUDEV_LIB_FILES

    include_directories(${INUDEV_INC})
    link_directories   (${INUDEV_LIB_DIR})

    list(APPEND B3D4_LINK_LIBS inudev)
endif()


## "b3d4_client" INCLUDE dependencies
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${B3DI_INC})  # This is for external module "b3d4client"
include_directories(${TRIMESH2_INC})  # This is for "b3di" header itself FIXME
# include_directories(${B3D_PRIVATE_INC_DIRS})

include_directories(${B3D_UTILS_INC_DIRS})
include_directories(${B3D_DEPTH_INC_DIRS})
include_directories(${B3D4_CLIENT_INC_DIRS})

link_directories(${B3D_DEPTH_LIB_DIRS})
link_directories(${B3DI_LIB_DIRS})

add_library(b3d4client SHARED
    ${B3D4_SRC_FILES}
    ${B3D4_HEADER_FILES}
)

if(ANDROID)
    find_library(log-lib log)
    list(APPEND B3D4_LINK_LIBS ${log-lib})
endif()

target_link_libraries(b3d4client
    ${B3D4_LINK_LIBS}
)


if (WIN32)
    # Set up Visual Studio file filters
    source_group("Public Headers" FILES ${B3D4_HEADER_FILES})
    source_group("Source Files"   FILES ${B3D4_SRC_FILES})

endif()

## Copy DLL to where Executables are
if(COPY_DLL)
    # Make sure the directory exist before copying
    set(DLL_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/b3d4client.dll")

    ADD_CUSTOM_COMMAND(TARGET b3d4client
              POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E make_directory ${EXE_DIR}
              COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_PATH} ${EXE_DIR}
              COMMENT "Copy b3d4client.dll"
    )
endif()
