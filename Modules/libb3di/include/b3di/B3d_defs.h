#pragma once

#include "opencv2/core.hpp"

namespace b3di {

	//const float INVALID_Z = 10000.0f;		// OpenCV reprojectImageTo3D set z to 10000 if the disparity is invalid
    const float INVALID_Z = 100000;         // Use this to indicate the z value is invalid. The value has to be larger than the max Z (65535)

//	const double DEPTH_UNIT_SCALE = 0.02;		// Deprecated. Use DepthUnitConverter instead
//	const double DEPTH_RESOLUTION = 0.02;		// in mm
//	const float MAX_ZVAL = (float)(65535 * DEPTH_RESOLUTION);			// in mm. Any z value greater than this will be truncated. MAX_ZVAL/DEPTH_RESOLUTION should be less than 65536 (ushort limit)

	// Min/max z values (in mm) for foreground (head) pixels; Values outside will be ignored
	const double MIN_FOREGROUND_Z = 150;
	//const double MAX_FOREGROUND_Z = 700;
	const double MAX_FOREGROUND_Z = 1300;		// The smallest depth unit is 0.02 to fit in 16-bit range
//	const double DEPTH_TO_GRAY_SCALE = 255.0*DEPTH_RESOLUTION / MAX_FOREGROUND_Z;

	// Face tracking range
	//const double TRACK_FACE_NEAR_RANGE = 250.0;
	//const double TRACK_FACE_FAR_RANGE = 380.0;

	const int DEF_JPEG_QUALITY = 80;
	//const int DEF_PNG_QUALITY = 9;

	// for saving C cam profile color images
	const int PROFILE_JPEG_QUALITY = 75;
	const int PROFILE_IMAGE_MAX_HEIGHT = 2048;

    const int PREVIEW_JPEG_QUALITY = 80;
    const int PREVIEW_IMG_MAX_HEIGHT = 640;
    const int PREVIEW_IMG_MARGIN = 64;

	// for face tracker
	// Ratio of the image width to search for a face in different camera formats
	//const double PORTRAIT_CAM_FACE_REGION = 0.8;
	//const double LANDSCAPE_CAM_FACE_REGION = 0.5;

	// Resize image for face detection to this height if it is greater than this
//	const int FACE_SEARCH_HEIGHT = 640;
//	const int FACE_SEARCH_HEIGHT = 960;

	// registerPnP parameters
	const int PNP_MAX_ITERATION = 1500;
	const double PNP_CONFIDENCE = 0.95;

	//const double Z_RANGE_MIN_OFFSET = 80;
	//const double Z_RANGE_MAX_OFFSET = 240;

	// ICP
	const double ICP_MAX_ERROR = 3.0;				// ICP max error for head pose tracking 
	const double ICP_LARGE_ERROR = 2.5;				// ICP error greater than this will require re-computation
	const double ICP_MERGE_ERROR = 3.0;				// ICP max error for merging depth
													// Max x rotation angle in degrees to be considered as a key frame
	const double MAX_KEY_X_ANGLE = 15.0;
	const double MAX_KEY_Y_ANGLE = 15.0;

	// use min vs max depth for z-buffering
	//#define KEEP_MIN_DEPTH

	// Triangulation of mesh
//	const float TRI_DEPTH_DIFF_THRESHOLD = 10.0f;
//	const float TRI_DEPTH_DIFF_THRESHOLD = 20.0f;

	// Head rotation in X when projecting to the cylindrical map
//	const double HEAD_TILT_ANGLE = -12;
	const double HEAD_TILT_ANGLE = -10;

	// For mesh simplification
	// 5..8 are good numbers
	// Smaller number yields more iterations (slower) and higher quality
	const double SIMPLIFY_AGRESSIVENESS = 5.0;

	// for triangulation; in mm; only connect vertices to form a face with edge length small than this
	//static const double EDGE_LEN_DIFF_THRESH = 20;		
	//static const double EDGE_LEN_DIFF_THRESH = 150;		// for triangulation; in mm
	static const double EDGE_LEN_DIFF_THRESH = 0;		// for triangulation; in mm

	// depth difference threshold for detecting connected region
	//static const double CONNTECTED_DEPTH_THRESH = 12.0;		// in mm

	// Cylindrical projection depth unit
	const double CYL_RADIUS_RESOLUTION = 0.01;	// in mm

	// The offset is from the face surface. Make sure there is enough room for facial features
	//CV_Assert((CYL_CENTER_OFFSET_BACK + 130) < CYL_MAX_RADIUS);

	const double CYL_THETA_RANGE = 360.0;		// in degrees

	// chroma key color to indicate background pixels
	const cv::Vec3b BG_KEY_COLOR = cv::Vec3b(0, 255, 0);

	//// starting head pose angle ranges in degrees (X, Y, Z)
	//const double MAX_START_POSE[] = { 2.0, 10.0, 5.0 };
	//const double MIN_START_POSE[] = { -15.0, -10.0, -5.0 };

	// starting face distance range
	//const double START_FACE_DISTANCE_RANGE[] = { 220.0, 450.0 };

	//// Required X and Y head rotation range for different scan modes
	//// X rotation: negative (look up), positive (look down)
	//// Y rotation: negative (turn left), positive (trun right)
	//const double FRONTAL_MIN_ROT[] = { 0.0, -45.0 };
	//const double FRONTAL_MAX_ROT[] = { 0.0, 45.0 };
	//const double THREEQ_MIN_ROT[] = { 0.0, -65.0 };
	//const double THREEQ_MAX_ROT[] = { 0.0, 65.0 };
	//const double FULL_MIN_ROT[] = { -30.0, -65.0 };
	//const double FULL_MAX_ROT[] = { 30.0, 65.0 };

	// define this for VIVO ToF data set processing
	//#define _VTOF

	// define this to use square cylindrical depth map and texture map
	#define _SQUARE_MAP

	// define this to use dome projection for the top of the cylindrical map
	// Best to use this only for _SQUARE_MAP
	#define _DOME_TOP

#ifdef _SQUARE_MAP
	const double CYL_PHI_RANGE = 540.0;		// in mm (for othorgraphic projection)
#else
	const double CYL_PHI_RANGE = 290.0;		// in mm (for othorgraphic projection)
#endif // _SQUARE_MAP

	const double CYL_CENTER_OFFSET_BACK = 90.0;		// z offset in mm from the face center to the center of the head
//	const double CYL_CENTER_OFFSET_FRONT = 80.0;		// z offset in mm from the face center to the tip of the nose (max)
	const double CYL_MIN_RADIUS = 10.0;				// in mm
//	const double CYL_MAX_RADIUS = CYL_CENTER_OFFSET_BACK + 150.0;				// in mm
//	const double CYL_MAX_RADIUS = CYL_CENTER_OFFSET_BACK + 120.0;				// in mm
    const double CYL_MAX_RADIUS = CYL_CENTER_OFFSET_BACK + 80.0;				// in mm

	// ADD_WATERMARK is now set in CMake. Do not define it. Use CMake instead
	// Define this to add a watermark to the texture map
	//#define ADD_WATERMARK
	const double WATERMARK_ALPHA = 0.15;

	const double B3D_LOGO_SIZE = 40;			// in mm

//	// Define this to enable 360 cylindrical head map in the horizontal direction
//	#define CYL_MAP_H_360
//
//	// define this to map Y coordinates to polar coordinates (spherical mapping)
//	// otherwise, map Y coordinates othographically (cylindrical mapping) 
//	// doesn't seem to work better so don't define this (use othorgraphic projection in Y)
//	//#define CYL_MAP_Y_POLAR
//
//#ifdef CYL_MAP_H_360
//	const double CYL_THETA_RANGE = 360.0;		// in degrees
//
//#ifdef CYL_MAP_Y_POLAR
//	const double CYL_PHI_RANGE = 180.0;		// in degrees
//#else
//
//#ifdef _SQUARE_MAP
//	const double CYL_PHI_RANGE = 580.0;		// in mm (for othorgraphic projection)
//#else
////	const double CYL_PHI_RANGE = 290.0;		// in mm (for othorgraphic projection)
//#endif // _SQUARE_MAP
//
//#endif // CYL_MAP_Y_POLAR
//
//#else
//	const double CYL_THETA_RANGE = 220.0;		// in degrees
//
//#ifdef CYL_MAP_Y_POLAR
//	const double CYL_PHI_RANGE = 120.0;		// in degrees
//#else
//	const double CYL_PHI_RANGE = 300.0;		// in mm (for othorgraphic projection)
//#endif // CYL_MAP_Y_POLAR
//
//#endif // CYL_MAP_360


	//#if defined(CYL_MAP_Y_POLAR)
	//// FIXME:
	//// Map both X and Y to polar coordinates (spherical map)
	//const double CYL_PHI_RESOLUTION = 7.0;		// in degree
	//const int CYL_HEIGHT = (int)(CYL_PHI_RESOLUTION * 180.0);
	//#elif defined(CYL_MAP_Y_ORTHO)
	//// Map Y with orthographic projection
	////const int CYL_HEIGHT = 720;		// need to cover the entire head (scaled by CYL_SCALE_Y)
	////const float CYL_SCALE_Y = 2.5f;		// 0.4 mm resolution in Y
	////	const int CYL_HEIGHT = 640;		// need to cover the entire head (scaled by CYL_SCALE_Y)
	////	const float CYL_SCALE_Y = 2.13f;
	//#else
	//// FIXME:
	//// Map Y with projective mapping
	//const int CYL_HEIGHT = 720;
	//const int CYL_CAM_FY = 680;
	//const double CYL_RADIUS_OFFSET = 300.0 - CYL_CENTER_OFFSET_BACK;
	//#endif



	// Define this to compute C cam head pose from M Cam for texture mappping
	// If this is undefined, C am head pose will be computed from time stamps
	// NOTE: this must be defined for now
	#define GET_HEADPOSE_FROM_M_CAM

	// Define this to use C_Cam for head pose tracking (unless C cam is not available)
	// Otherwise, use M_CAM for tracking
	//#define TRACKING_WITH_C_CAM


    // Define params for nostril enhancement
    const int NOSTRIL_PROFILE_INDEX = 1;

    // mask size for nostril filtering
    const int NOSTRIL_SPATIAL_FILTER_MASK_WIDTH = 5;
    const int NOSTRIL_SPATIAL_FILTER_MASK_HEIGHT = 3;

    // Define the rect to save nostril info on depth map
    // x, y, width, height
    // x, y is the ratio of depth map width and height, the unit is %
    // width and height are in pixel unit
    const float DEPTH_MAP_NOSTRIL_RECT_RATIO_X = 0.1f;
    const float DEPTH_MAP_NOSTRIL_RECT_RATIO_Y = 0.7f;
    const int DEPTH_MAP_NOSTRIL_RECT_WIDTH = 250;
    const int DEPTH_MAP_NOSTRIL_RECT_HEIGHT = 250;

    // depth unit of nostil data
    const float NOSTRIL_DATA_DEPTH_UNIT = 0.02f;

    // contrast of nostril
    const float NOSTRIL_CONTRAST = 0.8f;

    // nostril enhnacement in mm
    const float NOSTRIL_ENHANCEMENT_SHIFT_DISTANCE = 3.0f;

    // define attributes for arc buildmesh
    // indices of arc cameras
    enum ARC_CAMERA_INDEX
    {
        CENTER_CAM = 0,
        FIRST_RIGHT_CAM = 1,
        FIRST_LEFT_CAM = 2,
        LOWER_CAM = 3,
        UPPER_CAM = 4,
        SECOND_RIGHT_CAM = 5,
        SECOND_LEFT_CAM = 6,
        CAM_NUM = 7
    };

    // arc camera names
    const std::vector<std::string> ARC_CAMERA_NAMES{
        "c", "r1", "l1", "d1", "u1", "r2", "l2"
    };

    // arc still capture input images
    enum ARC_STILL_CAPTURE_INPUT_IMAGE {
        DEPTH_IMAGE = 0,
        COLOR_IMAGE = 1,
        CONFIDENCE_MAP = 2,
        RMSE_MAP = 3,
        IR_MASK = 4,
        IMAGE_NUM = 5
    };

} // namespace b3di
