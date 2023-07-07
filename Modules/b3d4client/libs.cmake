# Define Macros for building B3DSTEREO project
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# # Configure "B3D_DEPTH" module dependency
# if (NOT DEFINED B3D_DEPTH_DIR)
#     # This is a dependency, B3D_DEPTH_DIR/libs.cmake has to be included first
#     message(FATAL_ERROR "B3D_DEPTH_DIR is not defined")
#     # set(B3D_DEPTH_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../b3d_depth")
# endif()

# B3D_CAMERA_DIR should have be defined by main CMakeLists.txt
if (NOT DEFINED B3D4_CLIENT_DIR)
    set(B3D4_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR})
    message(WARNING "B3D4_CLIENT_DIR is not defined, current using: ${B3D4_CLIENT_DIR}")
endif()

### Configure Include Directories
set(B3D4_CLIENT_INC_DIRS ${B3D4_CLIENT_DIR}/include)
#set(B3D_CAMERA_INC_INTERNAL ${B3D_CAMERA_INC}/internal)

# set(UTIL_INC  ${B3D_DEPTH_DIR}/utils/include)
# set(B3D_CAMERA_INC_DIRS ${B3D_CAMERA_INC} ${UTIL_INC})

# link and include b3di
# set(B3DI_DIR "${B3DI_DIR}/libb3di")
# include("${B3DI_DIR}/libs.cmake")

if(ANDROID)
    set(B3D_CAMERA_LIB_DIR ${B3D_CAMERA_DIR}/jniLibs/${ANDROID_ABI})

	set(B3D_CAMERA_LINK_LIBRARIES inudev ${B3D_DEPTH_LIBS} log)# TODO: MOVE this internally?

    set(B3D4_CLIENT_LIBS b3d4client)
elseif(WIN32)
    # on Windows, we are building STATIC library,
    # need to specify linked SHARED libraries only???
    set(B3D4_CLIENT_LIBS b3d4client)

    # Variables below are only used when "b3d4client" is installed
    set(B3D4_CLIENT_LIB_DIR ${B3D_CAMERA_DIR}/libs)

    set(B3D4_CLIENT_LINK_LIBRARIES inudev ${B3D_DEPTH_LIBS})  # TODO: MOVE this internally?
endif()
