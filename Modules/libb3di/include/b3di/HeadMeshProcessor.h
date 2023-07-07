
#pragma once

#include <vector>

#include "B3d_defs.h"
#include "DepthProcessor.h"
#include "B3d_files.h"
//#include "DepthCamera.h"
#include "PolyMesh.h"
#include "ProgressTracker.h"
#include "FaceTracker.h"
#include "CameraContainer.h"
#include "ImageContainer.h"
#include "FrameStore.h"
#include "DepthCamProps.h"
#include "HeadMeshI.h"
#include "buildmesh_error.h"
#include "B3d_types.h"


struct BuildMesh_Input {

	BuildMesh_Input() {

		// These are kept for backward compatibility but they are not used anymore since buildMesh returns headMesh
		// instead of writing output to the disk
		// Output dirs and files
		outputDir = "/mergedepth/";
		headDepthFile = "headDepth.png";
		headCamFile = "headDepth_cam.yml";
		photoMapFilePrefix = "photoMap_";

		// Texture file and material names
		materialFileName = "merge3d.obj.mtl";
		textureFileName = "merge3d.jpg";
		makeupFileName = "makeup3d.jpg";
//		materialName = "material_0";			// NOT USED. Defined in buildMesh.cpp
		faceLandmarkFileName = "faceLandmarks.yml";			// face landmarks in world space (for face model fitting)
		faceLandmarkObjFileName = "faceLandmarks.obj.yml";	// face landmarks in OBJ space

		landmark3dObjFileName = "faceLandmarks.obj";		// 3d landmarks OBJ file
															// Generate only one version for now
		simplifyRatioScales = { 1.0 };
		merged3DFileNames = { "merge3d.obj" };
		makeup3DFileNames = { "makeup3d.obj" };

		// Output C cam files
		//camCOutDir = "/camC_all/";

		// Input params
		sourceImageScale = 1.0;
		meshSmoothness = 10;
		enhanceMesh = true;
		enhancementScale = 1.0;
		enhanceEyes = true;
        smoothingFactor = 0;

		numFramesToDetectEyeBlink = 3;

        // Not used so removed. targetVertices is now set in HeadMesh output
        // Mesh simplificaiton ratio
		//meshSimplifyRatio = 1;
		//meshTargetVertices = 31000;

		// Generate multiple versions of meshes with different simplification ratios. Multiply the above ratio by the following scales
		//simplifyRatioScales = { 1.0, 0.167, 0.033 };
		// Optional. Only needed to write out OBJ files. The length of the list must matches that of simplifyRatioScales
		//merged3DFileNames = { "merge3d.obj", "merge3d-mid.obj", "merge3d-low.obj" };

		// Set it to 0 (no rotation), -1 (rotate 90 degrees ccw) or 1 (rotate 90 cw) if the rotation is known
		// set imageCRotation to -2 to find the rotation automatically
		camCImageRotation = -2;

		makeupTypeId = -1;

		whitebalance = false;		// ignored

		cropFace = true;
		writeAll = false;

		scanMode = b3di::SCAN_FRONTAL;

		coordinateUnit = 1;			// Use 1 for mm; 10 for cm

        wait = 0;
        show = false;
        verbose = false;

		addLogo = false;
		addWatermark = false;

		textureCamIndex = 3;		// set to 3 to use C cam. Set to 2 to use M cam. -1 for no texture

        defaultTextureColor = cv::Scalar(200, 200, 200);


		//depthCamProps = b3di::DEPTHCAM_PROPS[b3di::DEPTHCAM_B3D2];
		// set the depth camera type used to capture the data set
		// depthCameraType = b3d::DepthCamera::DEVICE_B3D3;
		//depthProcConfig = b3di::DepthProcessor::DepthProcessorConfig();

		depthCameraType = b3di::DEPTHCAM_B3D2;

		//turningMode = b3di::AUTO_DETECT;		// 0: head turning; 1: camera rotating relative to the head (including chair rotating)
        turningMode = b3di::TURN_HEAD;		// 0: head turning; 1: camera rotating relative to the head (including chair rotating)

		// not used
		hdOutput = false;
		// not used
		meshRes = b3di::MESH_DEF;		// if MESH_DEF, use hdOutput to determine (for backward compatibility)

		// max nuumber of frames to use for texture computation (valid number is 5, 7 or 9) 
		// 5 will have 3 horizontal frames and 9 will have 7 horizontal frames
		maxNumTextureFrames = 9;

		mcTimeOffset = 0;

		recalibrate = 0;		// recalibration search range (0 to disable), 1 to search +/- 1 pixel
    };

	cv::String outputPath;  // HeadProcessor can output results to a different folder

	// Input data
	std::vector<b3di::CameraParams> inputCams;
	std::vector<b3di::FrameStorePtr> frameStores;
	std::shared_ptr<b3di::CameraContainer> headPoseContainerPtr;
	b3di::FaceTracker::FaceLandmarks faceLandmarks;
	std::vector<int> colorFrameIndices;		// indices of the color frames to use if known (leave empty to compute)

	//// Optional: container of ICP output camera params computed during trackHeadPose
	//b3di::CameraContainer icpEstContainer;
	//// Optional: face landmarks of the first C cam frame
	//b3di::FaceTracker::FaceLandmarks faceLandmarks;

	//// If inputCams and camImages are empty
	//// Load the data from dataPath
	//// Input data dir path
	cv::String dataPath;
	//// All the files below are relative to dataPath
	//// Captured Camera Images Setting:
	//cv::String camImagePrefixes[MAX_NUM_CAMS];
	//cv::String camPaths[MAX_NUM_CAMS];
	//cv::String camImagePaths[MAX_NUM_CAMS];
	////cv::String compuExtrinsicPath;  // Images Directory for Pre-Computing Extrinsic

//	cv::String compCamLPath;
////	cv::String depthLPath;
//	cv::String compFaceLFile;

	cv::String asmTrackerDataPath;		// stasm face tracker training data path. If empty, default paths will be searched

	// Cam C image orientation
	// 0: no rotation, 1: 90 CW, -1: 90 CCW, 2: 180, -2: auto detect (rotate CCW to portrait)
	int camCImageRotation;

	// Scale to be applied to source image (<1.0) Default is 1.0 (no scale)
	// Smaller number improves speed but lowers output resolution
	double sourceImageScale;

	// 0-10 (0 means no smoothing and 10 means max. smoothness)
	int meshSmoothness;
	// Enhanced smoothed mesh (applicable only if meshSmoothness not 0)
	bool enhanceMesh;
	double enhancementScale;		// applicable only if enhanceMesh is true. 1.0 is normal and. Larger number means stronger enhancement

    int smoothingFactor;    // Factor to be applied to meshSmoothness that controls the amount of smoothing. Value should be between 20-40. Larger value is smoother

	// Mesh simplification ratio 0-1 (0 means no simplification)
	//double meshSimplifyRatio;

    // Not used so removed. targetVertices is now set in HeadMesh output
	// Set mesh simplification target number of vertices (0 means no simplification)
	//int meshTargetVertices;

	// 0-21, -1 for no makeup
	int makeupTypeId;

	// enhance the eye regions using face landmarks
	bool enhanceEyes;

	// ignored
	// white balance the color texture map
	bool whitebalance;

	// Add watermark to the texture map
	bool addWatermark;

	// Index of the camera to be used for texture map computation 
	// 2: M cam, 3: C cam, -1: no texture
	int textureCamIndex;

    // defaultTexture color
    // used to gerate a single color image when compute texture failed
    cv::Scalar defaultTextureColor;

	// Crop the face model to remove the regions above and below the face
	// Set this to false to disable cropping
	bool cropFace;

	// scanning mode: 0: frontal, 1: 3/4, 2: full
	int scanMode;

	int turningMode;

	// OBJ output coordiante unit in mm (10 for cm, 1 for mm)
	int coordinateUnit;

	// Number of initial color frames to detect for eye blink
	// Search up to this number of frames to find one with most wide opened eyes
	// Set to <=1 to disable eye blink detection (and always use the first frame)
	// Default is 5 to search up to 5 frames
	int numFramesToDetectEyeBlink;


	// Write out all intermediat results to files (headDepth.png, etc.)
	bool writeAll;

	b3di::DepthCamType depthCameraType;
	//b3di::DepthCamProps depthCamProps;		// see DepthCamProps.h

	bool hdOutput;			// whether or not to generate HD output (deprecated. use meshRes instead)
	b3di::MESH_RES meshRes;

	// Output depth map
	cv::String outputDir;
	cv::String headDepthFile;
	cv::String headCamFile;

	// Generate multiple versions of meshes with different simplification ratios. Multiply the above ratio by the following scales
	std::vector<double> simplifyRatioScales;
	// Optional. For generating OBj files
	std::vector<cv::String> merged3DFileNames;
	std::vector<cv::String> makeup3DFileNames;

	cv::String faceLandmarkFileName;
	cv::String faceLandmarkObjFileName;
	cv::String landmark3dObjFileName;

	// For generating texture map
	cv::String materialFileName;
	cv::String textureFileName;
	cv::String makeupFileName;
	cv::String materialName;
	cv::String photoMapFilePrefix;

	// Output camera C files
	// ??? do we need this?
	cv::String camCOutDir;

	// b3d::DepthCamera::DeviceType depthCameraType;
	//b3di::DepthProcessor::DepthProcessorConfig depthProcConfig;

	bool addLogo;

	int mcTimeOffset;		// M/C camera frame time offset

	int maxNumTextureFrames;	// control how many frames to use for texture map generation (valid: 5,7,9)


	int recalibrate;		// recalibrate the cameras using the first set of images. the value defines the search range (1-10). 0 to disable recalibration.

    int wait;
    bool show;
    bool verbose;
};

struct BuildMesh_Output {

	BuildMesh_Output() {};

	std::vector<b3di::CameraParams> recalibCams;			// return recalibrated L/R cameras if recalibrate flag is not 0
	std::shared_ptr<b3di::HeadMeshI> headMeshPtr;

	std::vector<int> selectedColorFrameIndices;

	//std::vector<b3di::PolyMesh> polyMeshes;
	//cv::Mat textureMap;
	//cv::Mat makeupMap;
	//b3di::FaceTracker::FaceLandmarks faceLandmarks;
};

// input for Arc still capture
struct Arc_StillCapture_Input {
    std::string layoutFilePath;
    std::vector<std::vector<b3di::FrameStore>>* frameStoresPtr;
    const std::vector<b3di::CameraParams>* depthCamsPtr;
    const std::vector<b3di::CameraParams>* colorCamsPtr;
    std::string cacheFilePath;
    std::string landmarkDataPath;
    
    int smoothness = 5;
    std::string bitePlateFilePath;
    std::string targetLayout;
};

// output for Arc still capture
struct Arc_StillCapture_Output {
    std::shared_ptr<b3di::HeadMeshI> headMeshPtr;
    int validCams;
    bool bitePlateAligned = false;
    bool cameraXfsExported = false;
};

// input for Arc still capture
struct Arc_MotionCapture_Input {
    std::string layoutFilePath;
    std::string inputFolderPath;
    std::string cacheFilePath;
    std::string landmarkDataPath;

    int smoothness = 5;
    int targetDepthFrameNum = 40;
    float depthFrameTimeOut = 5000.0f;
    float loadKeyDataTimeOut = 15000.0f;
    float loadColorImageTimeOut = 15000.0f;
};

// output for Arc still capture
struct Arc_MotionCapture_Output {
    std::shared_ptr<b3di::HeadMeshI> headMeshPtr;
};


namespace b3di {

class HeadMeshProcessor {

public:

	// Process input to create a HeadMesh output
	// Return an error code of BUILDMESH_ERROR
	BUILDMESH_ERROR process(BuildMesh_Input& input, BuildMesh_Output& output);

    // overload for Arc still capture mode
    // Process input to create a HeadMesh output
    // Return an error code of BUILDMESH_ERROR
    BUILDMESH_ERROR process(const Arc_StillCapture_Input& input, Arc_StillCapture_Output& output);

    // overload for Arc motion capture mode
    // Process input to create a HeadMesh output
    // Return an error code of BUILDMESH_ERROR
    BUILDMESH_ERROR process(const Arc_MotionCapture_Input& input, Arc_MotionCapture_Output& output);
};

}	// namespace b3di

