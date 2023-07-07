#pragma once

#include <opencv2/core.hpp>
#include <vector>

#include "FaceTracker.h"
#include "EarTracker.h"
#include "CameraParams.h"
#include "PointCloud.h"
//#include "PolyMesh.h"
#include "B3d_types.h"
#include "TImage.h"


namespace b3di {

class FaceModelTemplate;

class HeadMeshI {
public:

	// Should be identical to those in SfsUtils.h
	enum MaterialMapIndex {
		NORMAL_MAP_INDEX,
		ROUGHNESS_MAP_INDEX,
		SCATTERING_MAP_INDEX,
        DISPLACEMENT_MAP_INDEX,
		MATERIAL_MAP_COUNT
	};

	enum MaterialMapFlag {
		NORMAL_MAP = 0x0001,				// surface normal map
		ROUGHNESS_MAP = 0x0002,			// surface roughness map (comptable with SketchFab)
		SCATTERING_MAP = 0x0004,			// sub-surface scattering map (comptable with SketchFab)
        DISPACEMENT_MAP = 0x0008        // displacement map
	};

	enum MeshMode {
		MESH_DEFAULT,					// For full head scans, this is the same as MESH_FULLHEAD. For frontal scans, this is the same as MESH_FRONTAL_EXT
		MESH_FULLHEAD_EXT,				// Full head mesh with extended neck (only available for full head scans)
		MESH_FULLHEAD,					// Full head mesh with short neck (only available for full head scans)
		MESH_FRONTAL_EXT,				// Frontal head mesh with extended neck
		MESH_FRONTAL						// Frontal only mesh
	};

	// output mesh vertex resolution
	enum MeshResolution {
		MESH_RES_MAX,					// max resolution. no simplication applied (500K-1M vertices)
		MESH_RES_HD,					// HD resolution (120-200K vertices)
		MESH_RES_SD,					// SD resolution (30-50K vertices)
		MESH_RES_LD						// LD resolution (3-7K vertices)
	};

    enum FeatureKey {
        FEATURE_MOUTH,              // mouth opening
        FEATURE_LEFT_EYE,
        FEATURE_RIGHT_EYE,
        FEATURE_FACE_CONTOUR
    };


    enum CoordSpace {
        CS_UNKNOWN,                 // unknown coordiate space
        CS_RIGHT_Y_UP,              // right-handed coordinate system where Y axis points up  (default)
        CS_RIGHT_Z_UP,              // right-handed coordinate system where Z axis points up
        CS_LEFT_Y_UP,               // left-handed coordinate system where Y axis points up
        CS_LEFT_Z_UP               // left-handed coordinate system where Z axis points up
    };

    enum JawScanType {
        TS_3SHAPE,
        TS_CARESTREAM,
        TS_ITERO,
        TS_EXOCAD
    };

    enum CBCTScanType {
        CB_ICAT,                        // i-CAT CBCT
        CB_PREXION
    };

	// A horizontal sequence of photo views
    enum PhotoViewIndex {
        PH_C0,             // center view
        PH_L1,             // Left view 1 (immediate left of the center view)
        PH_R1,             // Right view 1 (immediate right of the center view)
        PH_L2,             // Left view 2  (view between L1 and L3)
        PH_R2,             // Right view 2 (view between R1 and R3)
        PH_L3,             // Left view 3 (leftmost view)
        PH_R3              // Right view 3 (rightmost view)
    };

	// Profile images stored with the HeadMesh
	// It's usually a super set of Photo Views
	enum ProfileImageIndex {
		PR_C0,		// center-front
		PR_B1,		// center-down
		PR_T1,		// center-top
		PR_R1,		// Right 1
		PR_L1,		// Left 1
		PR_R2,		// Right 2
		PR_L2,		// Left 2
		PR_R3,		// Right 3
		PR_L3,		// Left 3
        PR_NUM      // Number of profile images
	};


	// You can add new types but do not change the order of the existing ones as they are persistent 
	enum EnhancementType {
		NO_ENHANCEMENT = 0,
		ENHANCE_BY_TEXTURE = 1,
		ENHANCE_BY_LANDMARKS = 2
	};


	// You can add new types but do not change the order of the existing ones as they are persistent 
	enum ConcealmentType {
		NO_CONCEALMENT = 0,
		CONCEAL_EYES = 1
	};

	HeadMeshI();
	virtual ~HeadMeshI();


	// The inputFile extension is .b3e, the file is assumed to be encrypted
	bool read(const cv::String& inputFile);

	// If outputFile extension is ".b3e", save the output file encrypted
	// Otherwise, the ouptut will be unencrypted and .b3d extension will be appended if it is not given
	bool write(const cv::String& outputFile) const;

	// get the frontal image
	cv::Mat getFrontalImage() const;
	cv::Size getFrontalImageSize() const;

	// imageIndex 0 is the same as frontal image
	cv::Mat getProfileImage(int imageIndex) const;
	cv::Mat getProfileImage(ProfileImageIndex imageIndex) const { return getProfileImage((int)imageIndex);  }

	int getProfileImageCount() const;
	// call getCameraToWorldXform of the returned CameraParams to get the head pose tranformation
	std::vector<b3di::CameraParams> getProfileHeadPoses() const;

    // return a photo view image of the head cooresponding to viewIndex
    // The image is cropped to include only the head region and the eyes are kept at the same height
    // accross all view images
    // Also returns faceLandmarks matching the returned image
    // Not all views are available. Return an empty image if the view image is not available
    cv::Mat getPhotoView(PhotoViewIndex viewIndex, std::vector<cv::Point>& faceLandmarks) const;

	// same as the above but also return the head pose 4x4 transformation relative to the center view
	cv::Mat getPhotoView(PhotoViewIndex viewIndex, std::vector<cv::Point>& faceLandmarks, trimesh::xform& headPose) const;

    // Export all photo views as jpeg images to a zip file
    // zipFilePath should be a zip file (*.zip)
    // The images will be named as "photo_c0.jpg", "photo_l1.jpg", "photo_l2.jpg", etc.
    // If addViewer is true, create an html slide viewer in the same directory to browse the images
    // captionText is the optional title text in the viewer and is only used if addViewer is true
	// if addAngleToFileName is true, the head yaw angle of each view will be added to the image file name. e.g. photo_l1_30.jpg to indicate the head is turned 30 degrees to the left.
    bool exportPhotoViews(const cv::String& zipFilePath, bool addViewer=false, const cv::String& captionText=cv::String(), bool addAngleToFileName=true) const;

    // Set the preview image of the mesh. Deprecated. use computePreviewImage instead.
    void setPreviewImage(const cv::Mat& previewImage);


    // Compute the preview image from the frontal image and depth map
    // setPreviewImage should not be used
    // profileImages, profileHeadPoses must be set first 
    // return false if the computation failed (missing required data)
    bool computePreviewImage(bool removeBg=true);

    // Return a preview image of the head mesh
    // If a preview image is not set, return the first profileImages
    const cv::Mat getPreviewImage() const;

	// Set the mesh output coordindate unit. Default is 1 mm. Set it to 10 for 1 cm.
	void setCoordinateUnit(int coordUnit);
	int getCoordinateUnit() const { return _coordUnit; }

	// Return true if a full head mesh is available
	bool hasFullHead() const;

	// NOTE: use getMesh(PolyMesh& pm, MeshResolution meshRes, ...) below if possible instead of setting targetVertices or simplifyRatio. 
	// This call may be deprecated in the near future.
	// Return a polyMesh
	// If simplifyRatio is not zero, the mesh will be simplifed by the ratio (<1.0)
	// If simplifyRatio is zero and targetNumVertices is not zero, the mesh will be simplified to have targetNumVertices vertices
	// If both targetVertices and simplifyRatio are zero, return an unsimplified mesh
	// meshMode specifies how to crop the mesh. MESH_FULL_HEAD is only available if the original scan has full head data
	// and the function will return false if the requested meshMode is not available
	// meshMode sets the cropping mode of the head mesh.
	// If the value is MESH_DEFAULT, the cropping depends on the scanning mode
	// For frontal scans, MESH_DEFAULT means MESH_FRONTAL.
	// For frontal extended scans, MESH_DEFAULT means MESH_FRONTAL_EXT.
	// For fullhead scans, MESH_DEFAULT means MESH_FULLHEAD.
	// If waterTight is true, the returned mesh will be watertight (currently only supported for full head meshes)
    // If meshMask is not empty, it should be a 8-bit mask where only non-zero pixels will be included in the output mesh.
    // meshMask should be the same size as the depth map
    // If addDisplacement is true, the displacement map, if available, will be baked in the mesh geometry
    // Otherwise, the displayment map can be retrieved separately with getMaterials
    // Set addDisplacement to false if you plan to render the mesh with a normal map or a dispacement map
    // Call setCoordSpace prior to this call. The vertex order and normal directions are affected by the coordindate space (right vs left handed)
	// set addDisplacement to true to refine nostril region
    // If maxEdgeLength is not zero, remove triangles with edge length > maxEdgeLength (in mm)
	// If pixelateEyes is true, the eye region will be pixelated to hide the identity of the person (apply to both the texture and the mesh)
    bool getMesh(PolyMesh& pm, int targetVetices = 0, double simplifyRatio = 0, MeshMode meshMode= MESH_DEFAULT, bool waterTight=false,
        const cv::Mat& meshMask=cv::Mat(), bool addDisplacement=true, double maxEdgeLength=0);

	// same as the above but use meshRes to automatically determine the simplication ratio based on the mesh mode
	// This call will also set texture map output size automatically to match the mesh resolution
	bool getMesh(PolyMesh& pm, MeshResolution meshRes, MeshMode meshMode = MESH_DEFAULT, bool waterTight = false,
		const cv::Mat& meshMask = cv::Mat(), bool addDisplacement = true, double maxEdgeLength = 0);

	// return a fitted PlolyMesh and its texture map
	// the returned texture map should be passed to exportMesh via altTexture
	// templateDirPath should be the path of the directory that contains a template.yml and other files
	// return 0 if fitting completed without error
	// return error code <= 255 for non-fatal error (fitting completed but may not be accurate, e.g. ear detection failed.)
	// return error code > 255 for fatal error (fitting cannot be completed)
	// for details of fitting error, see FaceModelTemplate.h
	int getFittedMesh(const cv::String& templateDirPath, b3di::PolyMesh& pm, cv::Mat& textureMap);

	// HB version with additional return args
	// Ony works for hbfrontal template
	// return print image and an alpha mask for the print area
	// if skinColors is not empty, select the best matched skinColor and color correct the output image
	// return an index to the best matched skin color
	int getFittedMeshHB(const cv::String& templateDirPath, b3di::PolyMesh& pm, cv::Mat& textureMap,
		cv::Size printSize, cv::Mat& printImage, cv::Mat& printMask, 
		std::vector<cv::Vec3b>& skinColorChoices, int& selectedColor);

	// return a fitted PolyMesh and its texture map created by blending facial regions from other head meshes into this one
	// the returned texture map should be passed to exportMesh via altTexture
	// faceParts should be a vector of 3 head meshe ptrs, corresponding to the nose, eyes, and mouth regions to blend from (order needs to be observed)
	// Set to empty point if a face part is not avaiable and the corresponding region will not be changed
	// see getFittedMesh regarding returned error code
	int getFittedMeshByBlending(const cv::String& templateDirPath, const std::vector<HeadMeshI*>& faceParts, b3di::PolyMesh& pm, cv::Mat& textureMap);

	// same as getFittedMeshByBlending but return a HeadMeshI instead
	int getHeadMeshByBlending(const cv::String& templateDirPath, const std::vector<HeadMeshI*>& faceParts, b3di::HeadMeshI& dstMesh);

	// return a fitted PolyMesh and its texture map with the face region in this mesh replaced with that from srcMesh
	// the returned texture map should be passed to exportMesh via altTexture
	// see getFittedMesh regarding returned error code
	int getFittedMeshWithFaceReplaced(const cv::String& templateDirPath, HeadMeshI& srcMesh, b3di::PolyMesh& pm, cv::Mat& textureMap);

	// same as getFittedMeshWithFaceReplaced but return a HeadMeshI instead
	int getHeadMeshWithFaceReplaced(const cv::String& templateDirPath, HeadMeshI& srcMesh, b3di::HeadMeshI& dstMesh);

    // Return a retangular plane registered with the head model using the frontal photo as texture map
    // backgroundOffset defines the offset in mm from the face toward the back of the head to place the background plane
    bool getPhotoBackground(PolyMesh& pm, cv::Mat& photoTexture, double backgroundOffset=75, PhotoViewIndex photoIndex=PH_C0) const;

    // Get photo+3D mesh
    // Experimental feature. Not fully working yet
    bool getPhotoMesh(PolyMesh& pm, cv::Mat& photoTexture) const;

	// Set the concealment processing type to be applied to the texture and the mesh to hide the identity of the person
	// if permanent is true, the concealment is applied to the original data and cannot be reversed
	// If permanent is false, the concealment is only applied to the output mesh and can be reversed.
	// return true if successful
	bool setConcealmentType(ConcealmentType type, bool permanent=false);

	// return the consealment type and if it is permanent or not
	ConcealmentType getConcealmentType(bool& isPermanent) const;

	//void setCropping(CropMode cropMode);
	//CropMode getCropping() const { return _cropMode; }

	//// The number of vertices is controlled by setSimplication
	//// vertices contains a list of vertex coordinates (x, y, z, nx, ny, nz, u, v)
	//// faces is a list of vertex indices. Each face contains 3 indices (triangles)
	//bool getMesh(std::vector<float>& vertices, std::vector<int>& faces);

	//// return vertex i in vertices
	//static MeshVertex getVertex(const std::vector<float>& vertices, int i) { return *((MeshVertex*)&vertices[i<<3]); }

	//// return a vector of 3 vertex indices of face i in faces
	//static cv::Vec3i getFaceVertices(const std::vector<int>& faces, int i) { return *((cv::Vec3i*)&faces[i * 3]);  }

    // If originalSize is false, the texture map returned is scalled to match setTextureMapOutputSize() if it is set
	// If skipProcesing is true, all post processing of the texture map, such as enhancements, sharpness, concealment, will be skipped regardless their settings
    // Otherwise, it returns the original texture map
	cv::Mat getTextureMap(bool originalSize=false, bool skipProcessing=false) const;

    // set confidence map
    void setConfidenceMap(const cv::Mat& confidenceMap);
    // get confidence map
    cv::Mat getConfidenceMap() const;

    // set _nonFilteringRegionMask
    void setNonFilteringRegionMask(const cv::Mat nonFilteringRegionMask);
    // get _nonFilteringRegionMask
    cv::Mat getNonFilteringRegionMask() const;
    
	// Get material maps
	// materialFlags indicate the type of material maps to return and can be ORed to return multiple files
	// e.g. use "NORMAL_MAP | ROUGHNESS_MAP | SCATTERING_MAP" to return three types of material maps
	// Use MaterialMapIndex to access the map in the returned materialMaps
	bool getMaterialMaps(std::vector<cv::Mat>& materialMaps,
		int materailFlags = NORMAL_MAP) const;

	// Return the face landmarks
	// The 2d landmarks should match the output texture map coordinate space, or the original texture map size (originalTextureSize is true)
	// The 3d landmarks should be points in the world space where if is relative to the L camera (origin)
	// if objSpace param is true, the 3d landmarks will be transformed to match the head's OBJ file space, where the origin is the center of the head
	b3di::FaceTracker::FaceLandmarks getLandmarks(bool objSpace=false, bool originalTextureSize=false) const;

    // same as the above but allows the landmarks to be returned in a different coordinate unit than the one set in setCoordinateUnit
	b3di::FaceTracker::FaceLandmarks getLandmarks(int coordUnit, bool objSpace=false, bool originalTextureSize=false) const;

	// Return extended face landmarks FT_EXT face tracker type
	b3di::FaceTracker::FaceLandmarks getExtendedLandmarks(bool originalTextureSize = false);

    // Return the ear landmarks if available
    // The 2d landmarks should match the output texture map coordinate space, or the original texture map size (originalTextureSize is true)
    // The 3d landmarks should be points in the world space where if is relative to the L camera (origin)
    // if objSpace param is true, the 3d landmarks will be transformed to match the head's OBJ file space, where the origin is the center of the head
    b3di::EarTracker::EarLandmarks getEarLandmarks(bool objSpace = false, bool originalTextureSize = false) const;

    // same as the above but allows the landmarks to be returned in a different coordinate unit than the one set in setCoordinateUnit
    b3di::EarTracker::EarLandmarks getEarLandmarks(int coordUnit, bool objSpace = false, bool originalTextureSize = false) const;

	// Detect ear landmarks and return the left/right ear rect and detection status
	// Use getEarLandmarks to get the detected landmarks
	// return false if the the detection cannot be completed (e.g. ear tracker not initialized)
	bool detectEarLandmarks(bool& hasLeft, bool& hasRight, cv::Rect& leftEarRect, cv::Rect& rightEarRect);

	// DEPRECATED. Use FaceMask::createContour instead
	// get a 3d face contour for a half-mask frame that covers the nose and mouth area
	// samplePointNum is the number of points requested for the contour
	// contourPoints is a vector of 3d points for the contour
	// contourNormals are the normal vectors for each of the contour points
    // widthLimit is the suface distance limit of side-mouth center with offset-side in mm
	// return true if the contour is computed successfully, or false if it fails
	bool getMaskFrameContour(std::vector<cv::Point3f>& contourPoints, std::vector<cv::Point3f>& contourNormals, 
        int samplePointNum = 200, float widthLimit = 140.0f);

    // Modify the texture map
	// if compress is true, the texture map will be stored as compressed jpeg
	void setTextureMap(const cv::Mat& textureMap, bool compress=false);

	// set depth enhancement types to be applied when computing a displacement map
	// the types can be ORed to have both on
	void setMeshEnhancement(EnhancementType enhancementTypes);
	EnhancementType getMeshEnhancement() const;

	// Set mesh smoothness. 
	// The value should be between 0 and 10, which indicates the amount of smoothing to be applied to the exported mesh 
	// This function is only applicable for newly created HeadMesh
	// It is ignored for legacy HeadMesh
	void setMeshSmoothness(int smoothness);
	int getMeshSmoothness() const;

	// return true if the mesh smoothness can be modified on export
	// return false if it cannot (legacy HeadMesh)
	bool canModifySmoothness() const;

	// NOTE: use getMesh to set mesh resolution, which will automatically set the texture map output size.
	// You should not call this direcdtly and this call may be deprecated in the near future
    // Texture and material maps will be resized to outputSize when exporting
    // If outputSize is Size(), the original texture map size will be used
    void setTextureMapOutputSize(cv::Size outputSize) {
        _textureOutputSize = outputSize;
    }

    // Get the output texture map size
    cv::Size getTextureMapOutputSize() const {
        return (_textureOutputSize == cv::Size()) ? getTextureMapOriginalSize() : _textureOutputSize;
    }

    // Get the original texture map size stored with the head mesh
    cv::Size getTextureMapOriginalSize() const {
		return _textureSize;
        //return _textureMap.size();
    }
	// Same as getTextureMapOriginalSize()
	cv::Size getTextureMapSize() const {
		return getTextureMapOriginalSize();
	}

	// Set the file path to load the logo file from
	// This is required if addLogo is true in exportMesh
	void setLogoFilePath(const cv::String& logoFilePath);

	// Export the PolyMesh with texture map to the outputDir
	// The export file type is determined by the file extension of fileName.
	// Currently, only .obj, .zip, .ply .ply.zip .stl .gltf, .glb are supported
	// The function will also export additional files necessary depending on the file format (e.g. .mtl, .jpg, etc.)
	// The additional files will have the same file name but different extensions.
    // Exported texture map size is controlled by setTextureMapOutputSize()
    // If addPhotoBackground is true, a photo background plane will be added to the export
	// if altTexture is not empty, export altTexture instead of the texture map stored in the head mesh (to save texture map returned from getFittedMesh)
	bool exportMesh(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, 
        bool addPhotoBackground = false, const cv::Mat& altTexture=cv::Mat());

	// Export material maps to outputDir as PNG files
	// materialFlags indicate the type of material maps to export and can be ORed to export multiple files
	// The output file names will match the material type (normalmap.png, roughness.png, scattering.png)
	// By default only normalmap.png will be saved
    // Exported material map size is controlled by setTextureMapOutputSize()
    bool exportMaterialMaps(const cv::String& outputDir, int materailFlags = NORMAL_MAP);

	// Write landmarks to outputDir
	// landmarkFileName should end with .yml
	// The 2d landmarks should match the texture map coordinate space
	// The 3d landmarks should be points in the world space where if is relative to the L camera (origin)
	// if objSpace param is true, the 3d landmarks will be transformed to match the head's OBJ file space, where the origin is the center of the head
	// In addition, if objSpace param is true and an objFileName is provided, an OBJ file constrcuted from the 3d landmark points will be written out
	bool writeLandmarks(const cv::String& outputDir, const cv::String& landmarkFileName,
		bool objSpace =true, const cv::String& objFileName="", bool saveExtendedLandmarks=true);

    // Write ear landmarks to outputDir with landmarFileName
	// earLmkModelDir is the ear landmark detector model file directory path
	// if autoDetect is true, automatically detect ear landmarks if they are not available
	// Ear landmarks will be written to the ouptut file regardless if they are available
	// The ear landmarks coordinates will be set to zero if they are not available
	// To avoid exporting empty ear landmarks, call hasEarLandmarks first 
	// and call detectEarlandmarks to determine if the ear landmarks can be detected
    bool writeEarLandmarks(const cv::String& outputDir, const cv::String& landmarkFileName,
        const cv::String& earLmkModelDir, const cv::String& objFileName);

	//// same as the above but look for earLmkModelDir in the default directory
 //   bool writeEarLandmarks(const cv::String& outputDir, const cv::String& landmarkFileName,
 //       const cv::String& objFileName, bool autoDetect = true);

	bool empty() const {
//		return _pc.empty();
		return _depthMap.empty();
	}

	void clear();

	ScanMode getScanMode() const { return _scanMode; }
	void setScanMode(ScanMode scanMode, bool headTurning = true) { _scanMode = scanMode; _headTurning = headTurning; }

	// return the turning mode during the scanning (TURN_CAMERA means there is no head motion during the capture)
	TurningMode getTurningMode() const { return _headTurning == 1 ? TURN_HEAD : TURN_CAMERA;  }

	// Get the depth camera type used to capture the mesh
	cv::String getDepthCameraType() const { return _depthCamType; }

	// Get the current file version
	// If the headMesh is loaded from a file with read, return the version of the file
	cv::String getFileVersion() const { return _fileVersion; }

	// return a white balanced texture map for targetWhite
	cv::Mat whitebalanceTexture(const cv::Scalar targetWhite = cv::Scalar(255));

	// return unfiltered depth map 
	const cv::Mat getDepthMap() const { return _depthMap; }
	b3di::CameraParams getMeshCamera() const { return _headCam; }

	// return filtered depth map
	// may trigger recomputation of filteredDepthMap
	cv::Mat getFilteredDepthMap();

    cv::Rect getHeadRect() const { return _headRect; }
    void setHeadRect(const cv::Rect& headRect) { _headRect = headRect; return; };

	// Return a crop mask and cropRect based on the current meshMode
	// if watertight is true, return a mask for watertight models (full heads only. ignored for other mesh modes)
	cv::Mat getCropMask(cv::Rect& cropRect, MeshMode meshMode, bool watertight = false) const;

    // set/get transformation to be applied to mesh vertices when returning a PolyMesh or PointCloud
    void setTransformation(const trimesh::xform& xf) { _headXform = xf; }
    trimesh::xform getTransformation() const { return _headXform;  }

    // Get the transformation to align this mesh to targetMesh and return the alignment error
    // sampleScale controls the sub-sampling of points for registration and should be >0 and <=1
    // For example, sampleScale 1 means use all points and 0.5 means using 25% of the points
    // If rigidOnly is true, only the more rigid part of the face is used in the alignment.
    // Set this to true if the facial expressions are very different between the two meshes.
    // Call setTransformation to set the transformation for aligning with targetMesh
    // Return false if the alignment fails
    bool getXformToMesh(const HeadMeshI& targetMesh, trimesh::xform& xf, double& alignError, double sampleScale = 0.5, bool rigidOnly = true);

    // Return a transformation that will transform the mesh to some known coordinate system
    // For unknown coordinate systems, use getXformToCoordSpace
    //static trimesh::xform getXformToCoordSystem(CoordSystem targetSys);

    // Return a transformation that will translate by the value in translation
    static trimesh::xform getTranslationXform(const cv::Vec3d& translation);

    // Return a transformation that will rotate by the value in rotation (XYZ angles in radians)
    static trimesh::xform getRotationXform(const cv::Vec3d& rotation, const cv::Point3d& rotationCenter=cv::Point3d());

    // Get the transformation to align to upper and lower jaw scans
    // The alignment accuracy is around 5 mm so manual alignment may be necessary to get more precise alignment
    // coordSpace defines the jaw scans' coordinate space.
    // If coordSpace is CS_UKNOWN, guessJawScanCoordSpace will be called to estimate and the output coordindate space will be returned.
    // Call setTransformation to set the transformation with the returned xf
    // Call setCoordSpace to set the coordinate space to match that of the target jaw scan
    // Return false if the alignment fails
    bool getXformToJawScan(const cv::String& upperJawScanFile, const cv::String& lowerJawScanFile, trimesh::xform& xf,
        CoordSpace& coordSpace);

    // Same as the above but instead of jawCoordSpace, takes an input transform that transforms the jaw to
    // the CS_RIGHT_Y_UP coordinate space
    // This allows an arbitray input jaw transformation if it is not in one of the known CoordSpaces
    bool getXformToJawScan(const cv::String& upperJawScanFile, const cv::String& lowerJawScanFile, trimesh::xform& xf,
        const trimesh::xform& jawCoordXf);

    // Replaced by getXformToJawScan
    //bool getXformToTeethScan(cv::String upperJawScanFile, cv::String lowerJawScanFile, trimesh::xform& xf,
    //    CoordSpace coordSpace= CS_RIGHT_Y_UP);

    // Guess the upper jaw scan file's coordinate space
    // Return false if it cannot guess (can't open the file, etc.)
    // Only support CS_RIGHT_Y_UP, CS_LEFT_Y_UP, CS_LEFT_Z_UP, CS_RIGHT_Z_UP currently
    static bool guessJawScanCoordSpace(const cv::String& jawFile, CoordSpace& coordSpace);

    // Guess the jaw scan's transformation to the default CS_RIGHT_Y_UP coordinate space
    // jawFile should be the upper jaw file but it is not availble, you can use lower jaw file
    // The returned transformation can be used as input to getXformToJawScan
    // Also return the coord space the jaw mode is in
    static bool guessJawScanCoordTransformation(const cv::String& jawFile, trimesh::xform& jawCoordXf,
        CoordSpace& coordSpace);

    // Return the jaw scan coordinate space given a jawScanType
    static CoordSpace getJawScanCoordSpace(JawScanType jawScanType);

    // Return a transformation that transforms from the default CS_RIGHT_Y_UP space to the targetCoordSpace
    static trimesh::xform getXformToCoordSpace(CoordSpace targetCoordSpace);

    // Return a transformation that transforms from the targetCoordSpace to the default CS_RIGHT_Y_UP
    static trimesh::xform getXformFromCoordSpace(CoordSpace targetCoordSpace);

    //// Same as the above but instead of specifying coordSpace, specifying the teeth scan type
    //bool getXformToTeethScan(cv::String upperJawScanFile, cv::String lowerJawScanFile, trimesh::xform& xf,
    //    TeethScanType teethScanType);

    // Return coordinate space given a CBCT scan type
    static CoordSpace getCBCTScanCoordSpace(CBCTScanType scanType);

    // Compute the transformation to align the mesh to a CBCT soft tissue scan file
    // coordSpace must be set to match that of the CBCT scan.
    // Call getCBCTScanCoordSpace get the coordinate space of a known CBCT scan type
    // Call setTransformation to set the transformation with the returned xf
    // Must call setCoordSpace to set the coordinate space to match that of the target CBCT scan
    // Return false if the alignment fails
    // Retrun the alignment error
    bool getXformToCBCTScan(const cv::String& cbctScanFile, trimesh::xform& xf, double& alignError, CoordSpace coordSpace= CS_RIGHT_Z_UP);

    // Return a 8-bit mask matching the depth map size where the pixel values inside the face feature are 0
    // and anything outside are 255. The featureMask can be passed to getMesh as meshMask
    // If inverted is true, the pixel values will be inverted (255 inside and 0 outside)
    // For example, to remove teeth from the mesh, you should set key to FEATURE_TEETH and inverted to false
    // You can call this function multiple times to add mutiple features by using the returned featureMask
    // as the input for the next call (e.g. to remove both eyes)
    bool getFeatureMask(FeatureKey key, cv::Mat& featureMask, bool inverted = false) const;

    // Get the mesh's coordinate space. The default is CS_RIGHT_Y_UP
    CoordSpace getCoordSpace() const;
    // Set the mesh's coordinate space
    void setCoordSpace(CoordSpace coordSpace);
	static cv::String coordSpaceToString(CoordSpace cs);
	static CoordSpace stringToCoordSpace(const cv::String& str);

    // DO NOT expose th function
    // this fuction should only be called in buildmesh
    // enhance nostril region from raw nostril points
    // save results in depth map and texture map
    bool enhanceNostrilRegion(std::vector<cv::Point3f>& rawNostrilPoints);

    // Create before and after images with jaw scans rendered in the after image
    // The head model is assumed to be aligned with the jaw scans already
    bool renderJawScansToFaceImage(const cv::String& upperJawScanFile,
        const cv::String& lowerJawScanFile,
        cv::Mat& beforeFaceImage, cv::Mat& afterFaceImage) const;

    // NOT DONE YET. Don't use.
    // Refine teeth area depth from jaw scan files
    // The head model is assumed to be aligned with the jaw scans already
    // Only the teeth area inside the lip line will be replaced with the jaw scan depth
    bool refineTeethDepthFromJawScans(const cv::String& upperJawScanFile,
        const cv::String& lowerJawScanFile);

	//void setMeshEnhancement(bool fromTexture, bool fromLandmarks) const;

	// get interpupillary distance
	// IPD can be computed from pupil center points (default)
	// or from the center of eye points (fromEyeCenters is true)
	double getIPD(bool fromEyeCenters=false) const;


	// return the default mesh mode based on the scanning data
	// this is the mode that will be use if you set mesh mode to MESH_DEFAULT when calling getMesh()
	MeshMode getDefaultMeshMode() const;

	// compute a texture map from profile images, depth map and head poses
	bool computeTextureMap(cv::Mat& textureMap);

    // set ear landmarks
    void setEarLandmarks(const b3di::EarTracker::EarLandmarks& earLandmarks);

    double getTextureSharpness() const { return _textureSharpness; };

    void setTextureSharpness(double sharpness);

	// compute extended face landmarks from input cylindrical depth map, face and ear landmarks
	static FaceTracker::FaceLandmarks getExtendedLandmarks(
		const cv::Mat& depthMap,
		const CameraParams& meshCam, cv::Rect headRect,
		const FaceTracker::FaceLandmarks& faceLandmarks,
		const EarTracker::EarLandmarks& earLandmarks
		);

protected:

    friend class LiplineEditor;
	friend class HeadMeshProcessor;
	friend class JawAlignment;
    friend class CylDepthFilter;
	friend class FaceModelTemplate;

	// Set multiple profile images of the head. The first one should be the same as the frontal image
	// The rest should be profile views of the head.
	// if compress is true, the image will be stored as compressed jpeg
	// these are called by buildMesh automatically
	void setProfileImages(const std::vector<cv::Mat>& profileImages, bool compress = false);
	void setProfileHeadPoses(std::vector<b3di::CameraParams>& headPoses);
	// Set the depth camera type used to capture the mesh
	void setDepthCameraType(cv::String& depthCamType) { _depthCamType = depthCamType; }

	void setLandmarks(const b3di::FaceTracker::FaceLandmarks& landmarks);

	// create a  head mesh from the input data
	// set depthMapSmoothness to -1 if the input detphMap is unsmoothed
	// otherwise, smoothness should be between 0 and 10 
	bool create(const cv::Mat& depthMap, const cv::Mat& textureMap,
		const CameraParams& headCamera,
		const cv::Rect& headRect = cv::Rect(), bool compressTexture = false, int depthMapSmoothness = -1);


    std::vector<cv::Point> getLiplineControlPoints() const;
    void setLiplineControlPoints(const std::vector<cv::Point>& lipPoints);
    static cv::Mat getFeatureMaskFromControlPoints(const std::vector<cv::Point>& controlPoints, cv::Size imageSize, 
        double pointScale=1.0, bool inverted=false);

	bool writeProps(const cv::String& filePath) const;
	bool readProps(const cv::String& filePath);
	bool exportObj(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo, const cv::Mat& altTexture = cv::Mat());
	bool exportZip(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo, const cv::Mat& altTexture = cv::Mat());
    bool exportStlZip(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo);
	bool exportStl(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo);
	bool exportGltf(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo, const cv::Mat& altTexture = cv::Mat());
	bool exportPlyZip(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo, const cv::Mat& altTexture = cv::Mat());
    bool exportPly(const PolyMesh& pm, const cv::String& outputDir, const cv::String& fileName, bool addLogo = false, const cv::Mat& altTexture = cv::Mat());


    // Return a PointCloud object of the mesh
    // contourOffset is the amount to bend inwards in mm for the mask cut contour (mainly for lip cut to bend the lip line inwards)
	// set watertight to true if the point cloud is for generating a watertight mesh (full head meshes only. ignored for others)
	// If sampleScale is not 1, resample the depth map to create point cloud.
    bool getPointCloud(PointCloud& pc, MeshMode meshMode = MESH_DEFAULT,
        const cv::Mat& meshMask = cv::Mat(), bool addDisplacement = true, int contourOffset =0, 
		bool watertight=false, double sampleScale=1.0);

    // same as above but does not call addNostrilEnhancementToPointCloud, which is not a const function
	// and also doesn't do filtering or mesh enhancements
    // the function is a const function and can be called with no side effects
    bool getPointCloud2(PointCloud& pc, MeshMode meshMode = MESH_DEFAULT,
        const cv::Mat& meshMask = cv::Mat()) const;

	//// write to a string buffer instead of a file
	//cv::String writePropsToBuffer() const;
	//// write to a string buffer instead of a file
	//bool readPropsFromBuffer(const cv::String& buffer);

	bool writeProps(cv::FileStorage& fs) const;
	bool readProps(const cv::FileStorage& fs);

    // add nostril enhancement to point cloud
    // this function is called in getmesh()
    bool addNostrilEnhancementToPointCloud(b3di::PointCloud& pc, const cv::Mat& filteredDepthMap, 
        const cv::Rect& headRect);

    // recompute uv for nostril region after nostril enhancement
    // this function is called in getmesh()
    void recomputeUVAfterNostrilEnhancement(b3di::PolyMesh& pm, const trimesh::xform& headXform, const double coordScale);

    // guess the coordinate space of jawPoints, which will be modified
    static bool guessJawScanCoordSpace(const std::vector<cv::Point3f>& jawPoints, CoordSpace& coordSpace);

	// Compute or recompute a displacement map to be applied to depthMap from texture map and/or from face landmarks
	// This function should be called if texture map or landmarks are modified
	cv::Mat computeDisplacementMap(const cv::Mat& depthMap) const;

	// smoothness ranges from 0 to 10 with 0 being least amount of filtering and 10 being the most (smoothest)
	// if smoothness is -1, it will only do noise removal without smoothing
	bool computeFilteredDepthMap(cv::Mat& filteredDepth, int smoothness);
    //b3di::FaceTracker::FaceLandmarks getProfileImageFaceLandmarks(int imageIndex) const;

	//Mat getDisplaymentMap();

	bool concealTextureMap(cv::Mat& textureMap) const;
	bool concealDepthMap(cv::Mat& depthMap) const;


	// Refine the interior (non-contour) cylindrial map landmarks using landmarks detected from the frontal image
	// Frontal image (_profileImages[0]) must be non-empty
	bool refineInteriorLandmarks();

	// Refine the exterior (contour) cylindrial map landmarks using depth data, skin detection and ear tracker
	// depthMap, textureMap, landmarks, headCam must be non-empty
	bool refineExteriorLandmarks();

	// Compute a depth map registered with a profile image identified by imageIndex
	// outputDepthMap size is determined automatically to minimize reprojection holes
	// It match not match the project image size
	// If meshMask is set, only project points inside the mask. See getMesh() for definition of meshMask
	// If depth32 is true, generate a 32-bit floating point depth map. Default is 16-bit fixed point depth map
	// setProfileImages and setProfileHeadPoses must have been called and imageIndex must be within the range
	// return true if outputDepthMap is computed successfully
	bool computeProfileDepthMap(int imageIndex, cv::Mat& outputDepthMap,
		const cv::Mat& meshMask = cv::Mat(),
		bool depth32 = false) const;

    cv::Rect getSrcHeadRect() const { return _srcHeadRect; }
    void setSrcHeadRect(const cv::Rect& srcHeadRect) { _srcHeadRect = srcHeadRect; return; };

	// compute extended landmarks from ASM face landmarks and ear landmarks
	// faceLmk and earLmk should have the same imageSize
	// headRect is the bounding rect of the face region and should match the landmark image size
	// filteredDepthMap size should match faceLmk and earLmk imageSize
	static void computeExtendedLandmarks2d(const cv::Rect& headRect, const FaceTracker::FaceLandmarks& faceLmk,
		const EarTracker::EarLandmarks& earLmk, const cv::Mat& filteredDepthMap, const CameraParams& depthCam,
		std::vector<cv::Point2f>& extendedLandmarks);

	int getFittedTemplateWithFaceReplaced(const cv::String& templateDirPath,
		HeadMeshI& faceMesh, FaceModelTemplate& fittedTemp, cv::Mat& fittedTexture);

	int getFittedTemplateByBlending(const cv::String& templateDirPath,
		const std::vector<HeadMeshI*>& faceParts, 
		FaceModelTemplate& fittedTemp, cv::Mat& fittedTexture);

	// set the head mesh's filteredDepthMap
	// should be called only by HeadMeshProcessor
	void setFilteredDepthMap(const cv::Mat& filteredDepthMap);


private:

	int getFittedTemplate(const cv::String& templateDirPath, 
		cv::Ptr<FaceModelTemplate>& fittedTemp, cv::Mat& fittedTexture, cv::Mat& textureAlpha);

	// output selectedFrameIndices
	enum TEX_FRAME_POS {
		TEX_FRM_00,
		TEX_FRM_B1,		// center-down
		TEX_FRM_T1,		// center-top
		TEX_FRM_R1,
		TEX_FRM_L1,
		TEX_FRM_R2,
		TEX_FRM_L2,
		TEX_FRM_R3,
		TEX_FRM_L3,
		TEX_FRM_COUNT
	};
	// disable caching of point cloud
	//PointCloud _pc;

    static cv::String coordSpace2String(CoordSpace cs);
    static CoordSpace string2CoordSpace(const cv::String& s);

	void initialize();

	//double _simplifyRatio;
	int _coordUnit;
	//int _targetVertexNum;
	cv::Rect _headRect;			// head rect set after filteredDepthMap is called
    cv::Rect _srcHeadRect;      // head rect computed by HeadBuilder and used as input to filterDepthMap
	TImage _textureImage;
	cv::Size _textureSize;
	cv::Mat _depthMap;


    cv::Mat _confidenceMap;
    cv::Mat _previewImage;
    cv::Mat _sharpRegionMask;
	//cv::Mat _frontalImage;
	b3di::CameraParams _headCam;
	b3di::FaceTracker::FaceLandmarks _landmarks;
    b3di::EarTracker::EarLandmarks _earLandmarks;
	cv::String _fileVersion;
	cv::String _depthCamType;
	cv::String _logoFilePath;
	std::vector<TImage> _profileImages;


	std::vector<b3di::CameraParams> _profileHeadPoses;
	//MeshMode _meshMode;
	ScanMode _scanMode;
	int _headTurning;		// 1 if the head is turning during the capture

    trimesh::xform _headXform;
    CoordSpace _coordSpace;

    cv::Size _textureOutputSize;
	//int _fullHeadBottomY;

	EnhancementType _enhancementType;

	ushort _concealmentType;		// type of consealment
	ushort _concealmentApplied;		// to indicate if consealment has been applied permanently

	int _depthMapSmoothness;	// amount of smoothness that has been applied to the depth map. -1 indicates no smoothing has been applied.
	int _meshSmoothness;	// 0 to 10 the amount of smoothing to be applied to the depth map when exporting to a mesh. -1 to disable all smoothing. Only applicable if _depthSmoothness is -1

	//double _cropZ;

    std::vector<cv::Point> _lipPoints;

    cv::Rect _nostrilRectOnNostrilView;
    cv::Rect _nostrilRectOnDepthMap;
    cv::Rect _nostrilRectOnTextureMap;

	// these are not persistent
    cv::Mat _nostrilViewPoints3D;
    cv::Mat _nostrilViewPointsUV;
	//cv::Mat _frontalImage;		// same as _profileImages[0] but uncompressed. computed on loading and not persistent

	cv::Mat _filteredDepthMap;		// this is computed on-the-fly (not persistent)
	//cv::Mat _displacementMap;		// computed on-the-fly (not persistent)

	cv::Mat _holeMask;			// not persistent. created by calling getFilteredDepthMap and is used by computeTextureMap

	cv::Ptr<FaceModelTemplate> _fittedTemplate;		// not persisent. created by getFittedMesh to cache the last fitted template so we don't have to recompute
	TImage _fittedTexture;		// not persistent. created by getFittedMesh to cache the last fitted texture map.
	int _fitError;				// not persisent. last fitting error

	double _textureSharpness; // amount of smoothness that has been applied to the depth map. -1 indicates no smoothing has been applied.
};

} // namespace b3di
