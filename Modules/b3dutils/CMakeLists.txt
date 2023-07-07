# Update the follow version string when new source files are added
# 0.1.0.6
set(B3D_UTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# message(FATAL_ERROR, "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# set(B3D_UTILS_INC_DIR "${B3D_UTILS_DIR}/include") # this is for other modules to use <b3dutils>
set(B3D_UTILS_INC "${B3D_UTILS_DIR}/include/b3dutils")
set(B3D_UTILS_SRC "${B3D_UTILS_DIR}/src")

# include("${B3D4_DIR}/libs.cmake")

file(GLOB B3D_UTILS_HEADER_FILES
    ${B3D_UTILS_INC}/*.h
	${B3D_UTILS_INC}/*.hpp
)

file(GLOB B3D_UTILS_SRC_FILES
    ${B3D_UTILS_SRC}/*.h
    ${B3D_UTILS_SRC}/*.cpp
)

include_directories(${B3D_UTILS_INC})

add_library(b3dutils SHARED
    ${B3D_UTILS_SRC_FILES}
    ${B3D_UTILS_HEADER_FILES}
)

if (WIN32)
    # Set up Visual Studio file filters
    source_group("Public Headers" FILES ${B3D_UTILS_HEADER_FILES})
    source_group("Source Files"   FILES ${B3D_UTILS_SRC_FILES})

endif()

## Copy DLL to where Executables are
if(COPY_DLL)
    # Make sure the directory exist before copying
    set(DLL_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/b3dutils.dll")
    # set(EXE_DIR  "${CMAKE_BINARY_DIR}/TestAppWin/$<CONFIGURATION>/")
    # set(LIB_DIR  "${B3D_UTILS_DIR}/lib/$<CONFIGURATION>/")

    ADD_CUSTOM_COMMAND(TARGET b3dutils
              POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E make_directory ${EXE_DIR}
              COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_PATH} ${EXE_DIR}
              COMMENT "Copy b3dutils.dll"
    )

    # ADD_CUSTOM_COMMAND(TARGET b3dutils
    #           POST_BUILD
    #           COMMAND ${CMAKE_COMMAND} -E make_directory ${LIB_DIR}
    #           COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_PATH} ${LIB_DIR}
    #           COMMENT "Copy b3dutils.dll"
    # )
endif()
