# Use extensions added by @EChen
add_definitions(-DECXT)

if(MSVC)
	add_definitions(/W0)
endif()

# Configure "trimesh2" module
set(TRIMESH2_INC "${LIBTRIMESH2_DIR}/include")
set(TRIMESH2_SRC "${LIBTRIMESH2_DIR}/libsrc")

## Configure header files for "trimesh2_lib"
file(GLOB TRIMESH2_HEADER_FILES
	${TRIMESH2_INC}/*.h
)

## Configure source files for "trimesh2_lib"
file(GLOB TRIMESH2_SRC_FILES
    ${TRIMESH2_SRC}/*.cc
)
### Some files can't be built on mobile platforms
if(NOT WIN32)
list(REMOVE_ITEM TRIMESH2_SRC_FILES "${TRIMESH2_SRC}/GLCamera.cc")
endif()

include_directories(${TRIMESH2_INC})

add_library(trimesh2
    ${TRIMESH2_SRC_FILES}
    ${TRIMESH2_HEADER_FILES}
)

# INSTALL trimesh2 module
set(TRIMESH2_HEADER_INSTALL_DIR "${B3DI_HEADER_INSTALL_DIR}/trimesh2")
INSTALL(FILES ${TRIMESH2_HEADER_FILES} DESTINATION ${TRIMESH2_HEADER_INSTALL_DIR})

if(ANDROID)
    INSTALL(TARGETS trimesh2 DESTINATION ${B3DI_LIB_INSTALL_DIR})

elseif(WIN32)
    INSTALL(TARGETS trimesh2
            DESTINATION ${B3DI_LIB_INSTALL_DIR}/Release
            CONFIGURATIONS Release)

    INSTALL(TARGETS trimesh2
            DESTINATION ${B3DI_LIB_INSTALL_DIR}/Debug
            CONFIGURATIONS Debug)
endif()
