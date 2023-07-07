# version 1.0.0.0
# only need this if compiling for WIN32

FIND_PACKAGE(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	add_definitions(-DUSE_OMP)
endif()

# Define app expiration date. Define EXP_YYYY 0 for no expiration
#add_definitions(-DEXP_YYYY=2021)
#add_definitions(-DEXP_YYYY=0)
# set expirate month. Note: Value should be 0-11. 0 for January and 11 for December
#add_definitions(-DEXP_MM=0)
# set day of the month (1-31)
#add_definitions(-DEXP_DD=1)

#### Configure dependencies
set(MODULES_DIR "${PROJECT_SOURCE_DIR}/Modules")

# Import "b3di" module
set(B3DI_DIR "${MODULES_DIR}/libb3di")
include("${B3DI_DIR}/libs.cmake")
# Exposes: B3DI_INC_DIRS, B3DI_LIB_DIRS, B3DI_LIBS, B3DI_DLLS, B3DI_DLL_DIR

# Import "b3dutils" module
set(B3D_UTILS_DIR "${MODULES_DIR}/b3dutils")
include("${B3D_UTILS_DIR}/libs.cmake")
# Exposes: B3D_UTILS_INC_DIRS, B3D_UTILS_LIB_DIRS, B3D_UTILS_LIBS

# Import "b3ddepth" module
set(B3D_DEPTH_DIR "${MODULES_DIR}/b3ddepth")
include("${B3D_DEPTH_DIR}/libs.cmake")
# Exposes: B3D_DEPTH_INC_DIRS, B3D_DEPTH_LIB_DIRS, B3D_DEPTH_LIBS

# Import "b3d4client" module
set(B3D4_CLIENT_DIR "${MODULES_DIR}/b3d4client")
include("${B3D4_CLIENT_DIR}/b3d4client.cmake")
# Exposes: B3D4_CLIENT_INC_DIRS, B3D4_CLIENT_LIB
# Exposes: B3DI_INC_DIRS, B3DI_LIB_DIRS, B3DI_LIBS

# Import "eigen" module
set(EIGEN_LIB_DIR "${MODULES_DIR}/thirdparty/eigen")

# Define ASM tracker file path
add_definitions(-DASM_TRACKER_DATA_PATH="${PROJECT_SOURCE_DIR}/Modules//thirdparty/stasm4.1.0/data/")

# Add include directories here
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${B3DI_INC_DIRS})
include_directories(${B3D_UTILS_INC_DIRS})
include_directories(${B3D_DEPTH_INC_DIRS})
include_directories(${B3D4_CLIENT_INC_DIRS})
include_directories(${EIGEN_LIB_DIR})

link_directories(${B3DI_LIB_DIRS})
link_directories(${B3D_DEPTH_LIB_DIRS})

# message(FATAL_ERROR "B3D_DEPTH_INC_DIRS ${B3D_DEPTH_INC_DIRS}")

# BUILD EXE TestStereo
ADD_EXECUTABLE(TestStereo teststereo_main.cpp)
TARGET_LINK_LIBRARIES(TestStereo ${B3D4_CLIENT_LIBS})

# BUILD EXE decode
ADD_EXECUTABLE(decode decode.cpp)
TARGET_LINK_LIBRARIES(decode ${B3D4_CLIENT_LIBS})

# BUILD EXE TestHeadPostTrack
ADD_EXECUTABLE(TestHeadPostTrack testHeadPostTrack_main.cpp)
TARGET_LINK_LIBRARIES(TestHeadPostTrack ${B3D4_CLIENT_LIBS})

# Restore Tool for B3D4 Manufacture
ADD_EXECUTABLE(RestoreTool restore_main.cpp)
#TARGET_LINK_LIBRARIES(RestoreTool ${B3D4_CLIENT_LIBS})

# Restore Tool for B3D4 Manufacture
ADD_EXECUTABLE(aecTool aec_main.cpp)
TARGET_LINK_LIBRARIES(aecTool ${B3D4_CLIENT_LIBS})

# async IR frame Tool for B3D4 test
ADD_EXECUTABLE(asyncframe asyncIRframe_main.cpp)
TARGET_LINK_LIBRARIES(asyncframe ${B3D4_CLIENT_LIBS})

# sync all frame Tool for 4DCapture test
ADD_EXECUTABLE(4DCaptureSync 4DCapture_main.cpp)
TARGET_LINK_LIBRARIES(4DCaptureSync ${B3D4_CLIENT_LIBS})