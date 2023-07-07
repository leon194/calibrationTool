#pragma once

namespace b3d4 {

    enum class B3D_ERROR_CATEGORY : int
    {
        B3D_NO_ERROR                        = 0,
        B3D_FILE_IO_ERROR                   = 1,
        B3D_CONNECTIVITY_ERROR              = 2,
        B3D_SENSOR_STREAMING_ERROR          = 3,
        B3D_DEPTH_COMPUTATION_ERROR         = 4,
        B3D_SINGLE_VIEW_MERGE_ERROR         = 5,
        B3D_SINGLE_CAMERA_BUILDMESH_ERROR   = 6,
        B3D_OTHER_ERROR                     = 7
    };

    enum class B3D_FILE_IO_ERROR : int
    {
        FILE_MISSING               = 101,
        FILE_READ_FAIL             = 102,
        FILE_WRITE_FAIL            = 103,
        FILE_FORMAT_NOT_SUPPORTED  = 104
    };

    enum class B3D_DEPTH_COMPUTATION_ERROR : int
    {
        INPUT_IMAGE_INVALID             = 401,
        INTPUT_CALIBATION_DATA_INVALID  = 402,
        INPUT_ROI_INVALID               = 403,
        INTPUT_RECT_MAP_INVALID         = 404,
        INPUT_CONFIG_INVALID            = 405
    };

    enum class B3D_SINGLE_VIEW_MERGE_ERROR : int
    {
        SETTINGS_INVALID                = 501,
        INPUT_DEPTH_INVALID             = 502,
        INPUT_CALIBRATION_DATA_INVALID  = 503,
        MERGE_FAILED                    = 504,
    };

    enum class B3D_SINGLE_CAMERA_BUILDMESH_ERROR : int
    {
        SETTINGS_INVALID                = 601,
        INPUT_IMAGE_INVALID             = 602,
        INPUT_CALIBRATION_DATA_INVALID  = 603,
        FACE_TRACKING_FAIL              = 604,
        HEAD_POSE_TRACKING_FAIL         = 605,
        MERGE_DEPTH_FAIL                = 606,
        TEXTURE_COMPUTATION_FAIL        = 607
    };

    enum class B3D_RECALIBRATION_ERROR : int
    {
        CALIB_NO_ERROR                  = 801,
        INPUT_IMAGE_INVALID             = 802,
        CALIB_SIZE_INVALID              = 803,
        CALIB_NO_CHANGE                 = 804
    };

    enum class B3D_FINDLANDMARK_ERROR : int
    {
        INIT_TRACKER_ERROR              = 901,
        FACE_NOT_FOUND                  = 902
    };

    enum class B3D_OPENCV_ERROR : int
    {
        OPENCV_ERROR                    = 1201
    };

    enum class B3D_OTHER_ERROR : int
    {
        OTHER_ERROR = 13001
    };

}