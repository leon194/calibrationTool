#pragma once


enum BUILDMESH_ERROR {
	BM_NO_ERROR = 0,

	// Non-error termination code
	BM_CANCELED = 1,

	// Error termination code
	BM_INPUT_ERROR = -1,				// missing or invalid input data
	BM_FACE_DETECTION_ERROR = -2,		// face detection error
	BM_SCAN_RANGE_ERROR = -3,			// input scan has insufficient head rotation range
	BM_TRACKING_ERROR = -4,			// Face tracking failure
	BM_OUTPUT_ERROR = -5,				// output error
	BM_HEAD_POSE_ERROR = -6,				// initial head pose is not in the range
	BM_DISTANCE_ERROR = -7,				// scanning distance is not in the range
	BM_PROCESS_ERROR = -8,				// build mesh processing error

	BM_UNKNOWN_ERROR = -99,
};
