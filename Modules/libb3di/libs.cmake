set(USE_EYELIKE   1)  # Set to use eyeLike library (0)
set(USE_WATERMARK 0)  # Uncomment this to add B3d watermark to output texture map
set(USE_MT        0)  # Set to use Meitu library (1) or not (0)

# add this to simulate compiling for iOS
#add_definitions(-DTARGET_OS_IPHONE)

# TODO: FACEPP_TRACKER option be turned into CMake build options
# if(NOT DEFINED WITH_FACEPP_TRACKER)
#     # Usually this should be defined by CMake options
#     # this is mainly for b3d_apps currently
#     set(WITH_FACEPP_TRACKER 1)
# endif()
# set(USE_ECXT 1)


# Make sure b3di library dependencies have been defined properly
if (NOT DEFINED B3DI_DIR)
    message(WARNING "B3DI_DIR not defined !")
    set(B3DI_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(B3DI_INC         "${B3DI_DIR}/include")
set(TRIMESH2_INC_DIR "${B3DI_INC}/trimesh2")

## Define TRIMESH2_INC, GLUIT_INC only for "b3d_apps"
set(TRIMESH2_INC  "${B3DI_DIR}/thirdparty/trimesh2/include")
set(GLUIT_INC     "${B3DI_DIR}/thirdparty/trimesh2/gluit")
set(B3DI_LIB_DIR  "${B3DI_DIR}/build/libs")
## This is for "b3d_apps"

# Exposed include directories used by b3di module
set(B3DI_INC_DIRS ${B3DI_INC} ${TRIMESH2_INC_DIR})


set(B3DI_OPENCV_LIBS
    opencv_core opencv_imgproc opencv_imgcodecs opencv_highgui
    opencv_features2d opencv_calib3d opencv_xfeatures2d opencv_objdetect
    opencv_dnn
)

### Configure facepp library
if(WITH_FACEPP_TRACKER)
if(WIN32)
    set(FACEPP_LIB_DIR "${B3DI_DIR}/libs")
    if(NOT EXISTS "${FACEPP_LIB_DIR}")
        # This fix is mainly for b3d_apps
        set(FACEPP_LIB_DIR "${B3DI_LIB_DIR}")  # defined above
    endif()

    # ToDo: choose v120 or v140 based on MSVS compiler version
    add_library(facepp STATIC IMPORTED)
    set_target_properties(facepp PROPERTIES IMPORTED_LOCATION_DEBUG   "${FACEPP_LIB_DIR}/Debug/facepp-sdk.lib")
    set_target_properties(facepp PROPERTIES IMPORTED_LOCATION_RELEASE "${FACEPP_LIB_DIR}/Release/facepp-sdk.lib")

elseif(ANDROID)
    set(FACEPP_LIB_DIR ${B3DI_DIR}/jniLibs/${ANDROID_ABI})
    add_library(facepp SHARED IMPORTED)

    # There two files, looks like only need libMegviiFacepp.so, don't need libmegface-new.so
    set_target_properties(facepp PROPERTIES IMPORTED_LOCATION_RELEASE ${FACEPP_LIB_DIR}/libMegviiFacepp.so)  #
endif()
endif()

set(B3DI_3RDPARTY_LIBS stasm trimesh2 sha256 eyeLike cppdes facepp)

if(NOT WITH_FACEPP_TRACKER)
    list(REMOVE_ITEM B3DI_3RDPARTY_LIBS facepp)
endif()


# Exposed library directories used by b3di module
set(B3DI_LIBS
    b3di
    ${B3DI_OPENCV_LIBS}
    ${B3DI_3RDPARTY_LIBS}
)

# Expose DLL library dependencies, callers of "b3di" need to know
# only for Windows platform
if(WIN32)
    set(B3DI_DLL_DIR "${B3DI_DIR}/dlls")
    file(GLOB B3DI_DLLS
        "${B3DI_DLL_DIR}/*.dll"
    )

    if(WITH_FACEPP_TRACKER)
        file(GLOB FACEPP_DLLS
            "${B3DI_DIR}/thirdparty/facepp/dll/x64/*.dll"
        )
        list(APPEND B3DI_DLLS ${FACEPP_DLLS})
    endif()

    # message(FATAL_ERROR "B3DI_DLL!!!!S: ${B3DI_DLLS}")
endif()


if(ANDROID)
set(B3DI_LIB_DIRS "${B3DI_DIR}/jniLibs/${ANDROID_ABI}")

elseif(WIN32)
set(B3DI_LIB_DIRS "${B3DI_DIR}/libs")

endif()
