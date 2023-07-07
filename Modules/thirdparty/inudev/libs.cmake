# InuDev version: 4.00.0009.87

# Make sure inudev library dependencies have been defined properly
if (NOT DEFINED INUDEV_DIR)
    message(WARNING "INUDEV_DIR not defined !")
    set(INUDEV_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

set(INUDEV_INC "${INUDEV_DIR}/include")


# if(ANDROID)
#     set(inudev )
# elseif(WIN32)
#     set(inudev optimized InuStreams_v140 debug InuStreams_v140d)
# endif()


### Configure Library Directories
if(WIN32)

    set(INUDEV_LIB_DIR ${INUDEV_DIR}/lib)

    # ToDo: choose v120 or v140 based on MSVS compiler version
    add_library(inudev STATIC IMPORTED)
    set_target_properties(inudev PROPERTIES IMPORTED_LOCATION_DEBUG   ${INUDEV_LIB_DIR}/InuStreams_v140d.lib)
    set_target_properties(inudev PROPERTIES IMPORTED_LOCATION_RELEASE ${INUDEV_LIB_DIR}/InuStreams_v140.lib)
elseif(ANDROID)

    set(INUDEV_LIB_DIR ${INUDEV_DIR}/jniLibs/${ANDROID_ABI})

    add_library(inudev SHARED IMPORTED)
    set_target_properties(inudev PROPERTIES IMPORTED_LOCATION_DEBUG   ${INUDEV_LIB_DIR}/libInuStreams.so)
    set_target_properties(inudev PROPERTIES IMPORTED_LOCATION_RELEASE ${INUDEV_LIB_DIR}/libInuStreams.so)

    # For installation purpose
    file(GLOB INUDEV_LIB_FILES
        ${INUDEV_LIB_DIR}/*.so
    )

    # libgnustl_shared.so is already included with Android system
    # add this line to avoid "more than one file was found with OS independent path" error
    list(REMOVE_ITEM INUDEV_LIB_FILES ${INUDEV_LIB_DIR}/libgnustl_shared.so)
endif()
