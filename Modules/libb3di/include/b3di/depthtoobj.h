
#pragma once

#include <vector>


#include "B3d_defs.h"
#include "CameraParams.h"
#include "PointCloud.h"


struct DepthToObj_Input {

	DepthToObj_Input() {

		simplifyRatio = 1.0f;
		targetNumVertices = 0;

		minZ = b3di::MIN_FOREGROUND_Z;				// in mm
		maxZ = b3di::MAX_FOREGROUND_Z;				// in mm

		coordinateUnit = 1.0;			// default in cm. Use 1 for mm
		//depthResolution = 0.1;

		edgeLenThresh = 5.0;
        wait = 0;
        show = false;
        verbose = false;

    };

	b3di::PointCloud pointCloud;		// input point cloud. If missing, depthImage is used
	cv::Mat depthImage;					// use depth image instead of point cloud
	b3di::CameraParams depthCam;	
	float simplifyRatio;				// mesh simplification ratio (<1.0). 0 to use targetNumberVertices.
	int targetNumVertices;			// mesh simplification target number of vertices (0 for no simplification)
	cv::String textureFileName;		// optional; must be inside outputDirPath
	cv::String landmarksFileName;		// optional face landmark file; must be inside outputDirPath
	cv::String logoFileName;		// optional logo file; must be inside outputDirPath
//	double depthResolution;			// 0.1 mm or 0.01 mm

	// Output file paths
	cv::String outputDirPath;
	cv::String objFileName;
	cv::String materialFileName;
	cv::String landmarksObjFileName;
	cv::String landmarksObjYmlFileName;

	// OBJ output coordiante unit in mm (10 for cm, 1 for mm)
	double coordinateUnit;
	double edgeLenThresh;		// edge length threshold for generating a face. Set this to eclude very elongated faces. (0 to generate all)

	double minZ, maxZ;		// min/max depth range (in mm)
    int wait;
    bool show;
    bool verbose;
};

struct DepthToObj_Output {

};

int depthToObj(DepthToObj_Input& input, DepthToObj_Output& output);

