# Define Macros for building b3d4client project
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# B3D_CAMERA_DIR should have be defined by main CMakeLists.txt
if (NOT DEFINED B3D4_CLIENT_DIR)
    set(B3D4_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR})
    message(WARNING "B3D4_CLIENT_DIR is not defined, current using: ${B3D4_CLIENT_DIR}")
endif()

### Configure Include Directories
set(B3D4_CLIENT_INC_DIR  ${B3D4_CLIENT_DIR}/include)  # For modules outside b3d4client
set(B3D4_CLIENT_INC_DIRS ${B3D4_CLIENT_DIR}/include)


if(ANDROID)
    # FIXME: link directories is a mess
    set(B3DI_LIB_DIR "${B3D4_CLIENT_DIR}/../libb3di/jniLibs/${ANDROID_ABI}")
    set(B3D4_CLIENT_LIB_DIRS ${B3DI_LIB_DIR})

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
