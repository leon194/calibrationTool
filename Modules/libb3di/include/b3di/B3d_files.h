/*M///////////////////////////////////////////////////////////////////////////////////////
//
// Define all head scanner data file and directory structure
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 1/27/2017	sec	created
//
//M*/
#pragma once

#include <opencv2/core.hpp>
#include "FileUtils.h"
#include "ImageContainer.h"

namespace b3di {

// B3D_PATH is different from SDK_PATH
// Bellus3D ( this is B3D_PATH )
//   SDK    ( this is SDK_PATH )
//   Output
#ifdef WIN32
    const cv::String DEF_B3D_PATH = "../../Bellus3d/FaceCamera/";
#elif defined(ANDROID)
    const cv::String DEF_B3D_PATH = "/storage/emulated/0/Bellus3d/FaceCamera/";
#elif defined(TARGET_OS_IPHONE)
    const cv::String DEF_B3D_PATH = "IPHX_TBD";
#else
    const cv::String DEF_B3D_PATH = "NULL";
#endif

// Bellus3D SDK folder structure
const cv::String SDK_DIRECTORY = "SDK/";
const cv::String OUTPUT_DIRECTORY = "Output/";
const cv::String CAPTURE_DIRECTORY = "CaptureData/";
//const cv::String CONFIG_FILE = "Configuration/config.yml";  // this is the config file for SDK

// Max number of cam image folders
const int MAX_NUM_CAMS = 6;

const int CAM_L_INDEX = 0;
const int CAM_R_INDEX = 1;
const int CAM_M_INDEX = 2;
const int CAM_C_INDEX = 3;
const int CAM_P_INDEX = 4;
const int CAM_D_INDEX = 5;


const cv::String CALIB_FILES[MAX_NUM_CAMS] = { "leftCam.yml", "rightCam.yml", "midCam.yml", "devCam.yml", "devCam_computed.yml", "depthCam.yml" };
//static const cv::String CALIB_DEV_CAM = "devCam_computed.yml";

const cv::String EAR_TRACKER_DIR = "Thirdparty/stasm4.1.0/data/";
const cv::String FPP_TRACKER_DIR = "Thirdparty/facepp/data/";
const cv::String ASM_TRACKER_DIR = "Thirdparty/stasm4.1.0/data/";
const cv::String CALIB_USER_DIR  = "UserFiles/CalibrationFiles/";
const cv::String CALIB_DEVICECAM_DIR = "Configuration/DeviceCamera/CalibrationData/";

const cv::String B3DCAMERA_TYPES_DIR = "Configuration/B3DCamera/";
const cv::String DEVICECAMERA_TYPES_DIR = "Configuration/DeviceCamera/";

const cv::String DEVICECAMERA_VERSION_FILE = "version.yml";

const cv::String HEADSCANNER_OUTPUT_DIR = "CaptureData/";  // was Output

//  --------  Path relative to Session Directory  --------  //
const cv::String CALIB_DATA_DIR = "CalibrationFiles/";

const cv::String RECALIB_DATA_DIR = "UserFiles/";

const cv::String CAMERA_DIR = "Camera/";

const cv::String CAMERA_IMAGE_DIR[MAX_NUM_CAMS] = { "L/", "R/", "M/", "C/", "P/", "D/" };
const cv::String CAMERA_IMAGE_PREFIX[MAX_NUM_CAMS] = { "L_", "R_", "M_", "C_", "P_", "D_" };
//static const cv::String CAMERA_IMAGE_SUFFIX[MAX_NUM_CAMS] = { ".bmp", ".bmp", ".jpg", ".jpg", ".jpg", ".png" };
// Use PNG for IR images
const cv::String CAMERA_IMAGE_SUFFIX[MAX_NUM_CAMS] = { ".png", ".png", ".jpg", ".jpg", ".jpg", ".png" };

const b3di::ImageContainer::ImageDataType CAMERA_IMAGE_TYPE[MAX_NUM_CAMS] =
{
    b3di::ImageContainer::IMAGE_GRAY, // L
    b3di::ImageContainer::IMAGE_GRAY, // R
    b3di::ImageContainer::IMAGE_RGB,  // M
    b3di::ImageContainer::IMAGE_RGB,  // C
    b3di::ImageContainer::IMAGE_RGB,  // P (ignored)
    b3di::ImageContainer::IMAGE_DEPTH // D (will be used soon)
};

const cv::String CAPTURE_METADAT_FILE = "Camera/metadata.yml";

const cv::String HEADTRACKER_OUTPUT_DIR = "Camera/poseL/";
const cv::String HEADTRACKER_OUTPUT_FACE_FILE = "Camera/faceM.yml";
const cv::String SELECTED_COLOR_FRAMES = "Camera/selectedFrames.yml";

const cv::String HEADSCANNER_OUTPUT_DEPTH_FILE = "headDepth.png";
const cv::String HEADSCANNER_OUTPUT_CAMERA_FILE = "headDepth_cam.yml";

const cv::String HEADSCANNER_OUTPUT_OBJ_FILE = "head3d.obj";
const cv::String HEADSCANNER_OUTPUT_MTL_FILE = "head3d.obj.mtl";
const cv::String HEADSCANNER_OUTPUT_TEX_FILE = "head3d.jpg";

const cv::String HEADSCANNER_OUTPUT_FACE_FILE = "faceLandmarks.yml";

const cv::String HEADPROCESSOR_LOG_FILE = "headProcessor.log";

const cv::String HEADSCANNING_LOG_FILE = "headScanner.log";

const cv::String HEADPROCESSOR_OUTPUT_SD_DIR = "Output_sd/";
//static const cv::String HEADPROCESSOR_OUTPUT_MD_DIR = "Output_md/";
// Change MD output dir to Output_hd since we don't process in HD
//const cv::String HEADPROCESSOR_OUTPUT_MD_DIR = "Output_hd/";
const cv::String HEADPROCESSOR_OUTPUT_HD_DIR = "Output_hd/";
const cv::String HEADPROCESSOR_OUTPUT_LD_DIR = "Output_ld/";

const cv::String FACE_ID_LOG_FILE = "faceID.log";


}
