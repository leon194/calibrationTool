
# Configure 3rd party libraries
if (NOT DEFINED THIRDPARTY_DIR)
    message(WARNING "THIRDPARTY_DIR is not defined before including thirdparty/libs.cmake")
    set(THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
endif()

set(LIBSTASM_DIR    "${THIRDPARTY_DIR}/stasm4.1.0/stasm")
set(LIBEYELIKE_DIR  "${THIRDPARTY_DIR}/eyeLike/src")
set(LIBLINEAR_DIR   "${THIRDPARTY_DIR}/liblinear")
set(LIBPOLYPART_DIR "${THIRDPARTY_DIR}/polypartition")
set(LIBTRIMESH2_DIR "${THIRDPARTY_DIR}/trimesh2")
set(LIBSHA256_DIR   "${THIRDPARTY_DIR}/sha256")
set(LIBCPPDES_DIR   "${THIRDPARTY_DIR}/cppDES/cppDES")

## This "gluit" module is mainly for b3d_apps
set(LIBGLUIT_DIR "${THIRDPARTY_DIR}/trimesh2/gluit")

## Configure include directories for b3di
set(STASM_INC     "${LIBSTASM_DIR}")
set(EYELIKE_INC   "${LIBEYELIKE_DIR}")
set(LIBLINEAR_INC "${LIBLINEAR_DIR}")
set(POLYPART_INC  "${LIBPOLYPART_DIR}")
set(TRIMESH2_INC  "${LIBTRIMESH2_DIR}/include")
set(SHA256_INC    "${LIBSHA256_DIR}")
set(CPPDES_INC    "${LIBCPPDES_DIR}")


### Header only libraries
set(SIMPLIFY_INC    "${THIRDPARTY_DIR}/simplify/src.cmd")
set(PERSISTENCE_INC "${THIRDPARTY_DIR}/Persistence1D/src")
set(TINYGLTF_INC    "${THIRDPARTY_DIR}/tinygltf")
set(MINIZ_INC       "${THIRDPARTY_DIR}/miniz-cpp")


## ??? only need this if compiling for WIN32
set(LIBEIGEN_DIR "${THIRDPARTY_DIR}/eigen")
set(EIGEN_INC    "${THIRDPARTY_DIR}/eigen")


## Face++ tracker "facepp" module
set(LIBFACEPP_DIR   "${THIRDPARTY_DIR}/facepp")
if(ANDROID)
set(FACEPP_INC      "${LIBFACEPP_DIR}/include/android")
elseif(WIN32)
set(FACEPP_INC      "${LIBFACEPP_DIR}/include/windows")
endif()
