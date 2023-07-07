#pragma once

#include <vector>

#include <opencv2/core.hpp>
#include "PolyMesh.h"
#include "CameraParams.h"
#include "HeadMeshI.h"
#include "FaceTracker.h"

namespace b3di {

static const cv::String TEMP_YML_FILE = "template.yml";
static const cv::String TEMP_3D_FILE = "template3d.obj";
static const cv::String MESHCAM_FILE = "meshCam.yml";

class FaceModelTemplate {

public:

	// model fitting error codes 
	enum FitError {
		FIT_NO_ERR = 0,

		// non fatal errors (fitting completed but may include these errors
		FIT_ERR_FULL_HEAD = 0x0001,
		FIT_ERR_LEFT_EAR_RECT = 0x0002,
		FIT_ERR_RIGHT_EAR_RECT = 0x0004,

		// fatal errors (fitting cannot complete due to these errors)
		FIT_ERR_FACE_LMS = 0x0100,
		FIT_ERR_EAR_LMS = 0x0200,
		FIT_ERR_INVALID_INPUT = 0x0400
	};

	enum FitScale {
		FIT_WIDTH,
		FIT_HEIGHT,
		FIT_AVERAGE
	};


	enum BlendType {
		BLD_NONE,			// no blending
		BLD_NOSE,			// blend the nose region
		BLD_EYES,			// blend the eye region
		BLD_MOUTH,			// blend the mouth region
		BLD_FACE			// blend the face region
	};


	FaceModelTemplate() {};

	// call create instead
	//// construt from a template yml file
	//FaceModelTemplate(const cv::String& tempFile, const cv::String& tempObjFile=cv::String()) {
	//	read(tempFile, tempObjFile);
	//}
	virtual ~FaceModelTemplate() {};

	// return the template type of templateFile
	static cv::String getTemplateFileType(const cv::String& templateFile);

	// create a ptr to FaceModelTemplate from templateType
	static cv::Ptr<FaceModelTemplate> newTemplatePtr(const cv::String& templateType);

	// construct a new Ptr to the template from a file
	// same as read but create a new object and return its Ptr instead
	static cv::Ptr<FaceModelTemplate> create(const cv::String& tempYmlFile, const cv::String& tempObjFile=cv::String());
	
	// create from default TEMP_YML_FILE and TEMP_3D_FILE in tempDir
	static cv::Ptr<FaceModelTemplate> createFromDir(const cv::String& tempDir);

	// if complete is false, write only data unique to this template and not in the original template
	// otherwise, wirte out all data, including those not changed
	virtual bool write(const cv::String& tempFile, bool complete =false) const;

	virtual bool empty() const {
		return landmarkVertices.empty(); 
	}

	// return true if the template is complete (with full topology) or partial with only coordinates
	// partial template depends on originalTemplate ptr for other info
	virtual bool isComplete() const {
		return !landmarkFaces.empty();
	}

	// return a copy
	virtual FaceModelTemplate clone() const;

	// clear the template
	virtual void clear();


	// create a fitted template from a head mesh and an original model template 
	// if computeTexture is true, also return a normalized texture map
	// return error code of FIT_ERROR. 
	// error code of FIT_ERR_FULL_HEAD, FIT_ERR_LEFT_EAR_RECT, FIT_ERR_RIGHT_EAR_RECT will not terminate the fitting
	// and a fitted template will still be created
	// error code of FIT_ERR_FACE_LMS or FIT_ERR_EAR_LMS will terminate the fitting
	// the returned template is a partial template and will set the originalTemplate field
	static FitError createFitted( 
		b3di::HeadMeshI& headMesh, const cv::Ptr<FaceModelTemplate>& originalTempPtr, 
		cv::Ptr<FaceModelTemplate>& fittedTemp,
		bool computeTexture, cv::Mat& textureMap);

	// compute fitted texture map from srcTexture
	virtual void computeFittedTextureMap(const cv::Mat& srcTexture, cv::Mat& fittedTexture) const;

	// Blend this template to targetTemp and save the blended result to targetTemp
	// alpha is a value between 0 and 1 that controls the overall blending
	// alpha=1 will keep the source and alpha=0 will keep the target
	virtual void blendTo(FaceModelTemplate& targetTemp, float alpha) const;

	// Blend this template to targetTemp and save the blended result to targetTemp
	// alphaMap is a 8-bit map for local blending controls. 
	// alphaMap 255 keeps the source and 0 keeps the target
	// alphaMap should be computed in the source template space
	virtual void blendTo(FaceModelTemplate& targetTemp, BlendType blendType) const;

	// blend srcTexture to dstTexture and return the blended texture in dstTexture
	// alpha is a value between 0 and 1 that controls the blending
	// alpha=1 will keep the source and alpha=0 will keep the target
	virtual void blendTexture(const cv::Mat& srcTexture, cv::Mat& dstTexture, float alpha);

	// blend srcTexture to dstTexture and return the blended texture in dstTexture
	// alphaMap is a 8-bit map for local blending controls
	// alphaMap 255 keeps the source and 0 keeps the target
	// alphaMap should be computed in the source template space
	virtual void blendTexture(const cv::Mat& srcTexture, cv::Mat& dstTexture, BlendType blendType);

	// return a PolyMesh from the template data and its meshCam
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// if baseShape is true, return the mesh formed by landmarks triangles only
	// If interiorFacesOnly is true, return only mesh faces that belong to interior landmark triangles
	virtual void getMesh(const b3di::CameraParams& meshCam, b3di::PolyMesh& pm,
		bool baseShape =false) const;

	// compute per vertex alpha from alphaMap (in cylindrical space)
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	virtual void computeVertexAlpha(const cv::Mat& alphaMap, std::vector<float>& vertexAlpha) const;
	
	/*** DRWING FUNCTIONS for DEBUGGING. ***/
	// draw landmark vertices and faces on targetImage
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// if fillFaces is true, fill the landmark faces with random colors
	virtual void drawLandmarks(cv::Mat& targetImage, bool fillFaces=false) const;

	// draw mesh vertices on targetImage.
	// if drawDepth is true, color each vertex with its depth value (if available)
	// otherwise, draw with a random color assigned to it enclosing landmark triangle
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// If interiorFacesOnly is true, draw only vertices that belong to interior landmark faces (not containing border edges)
	virtual void drawMeshVertices(cv::Mat& targetImage, bool drawDepth=false, bool interiorFacesOnly=false) const;

	// draw all mesh faces. If fillColor is set, fill each face with the color
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	virtual void drawMeshFaces(cv::Mat& targetImage, cv::Scalar fillColor) const;

	// draw only mesh vertices in selectedVertices
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	void drawSelectedVertices(cv::Mat& targetImage,
		const std::vector<int>& selectedVertices) const;

	// draw only frontal landmarks included mesh faces. If fillColor is set, fill each face with the color
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	virtual void drawFrontalLandmarksFaces(cv::Mat& targetImage, cv::Scalar fillColor) const;

	// return face landmarks 2d points from the template
	// TODO: the 3d landmarks are not returned currently
	// the 2d landmarks are scaled to match imageSize if set
	virtual void getFaceLandmarks(FaceTracker::FaceLandmarks& lms, cv::Size imageSize=cv::Size()) const;

	// return all mesh vertices as 3d points in the cylindrical space
	void getMeshVertices(std::vector<cv::Point3f>& meshVerts, float sx=1.0f, float sy=1.0f, 
		bool baseShape = false) const;

	// Return indices to all interior landmrk vertices
	// interior landmarks are landmarks that don't lie of a cylindrical map border
	virtual void getLandmarkVertexIndices(std::vector<int>& indices, bool interiorOnly=false) const;
	// Return indices to all mesh vertices
	virtual void getMeshVertexIndices(std::vector<int>& indices, bool interiorOnly = false) const;

	// return a list of interior landmark vertices
	// interior landmarks are landmarks that don't lie of a cylindrical map border
	virtual void getLandmarkVertices(std::vector<cv::Point3f>& vertices, bool interiorOnly = false) const;
	// for templates with 2d landmarks
	virtual void getLandmarkVertices2d(std::vector<cv::Point2f>& landmarkVerts, bool interiorOnly = false) const;

	// set landmark vertex points
	virtual void setLandmarkVertices(const std::vector<cv::Point3f>& vertices, bool interiorOnly = false);
	// for templates with 2d landmarks
	virtual void setLandmarkVertices2d(const std::vector<cv::Point2f>& vertices, bool interiorOnly = false);

	// return depth values of mesh vertices
	virtual void getVertexDepth(std::vector<float>& vertDepth, bool interiorOnly = false) const;

	// get/set depth offset values from the landmark triangle surface for mesh vertices
	virtual void getVertexDepthOffsets(std::vector<float>& depthVals, bool interiorOnly = false) const;
	virtual void setVertexDepthOffsets(const std::vector<float>& depthVals, bool interiorOnly = false);

	// return true if the vertex index is an edge landmark
	static bool isEdgeLandmark(int vertIndex);

	// Template data. Should be read only
	// TODO: add access functions 
	cv::String templateType;
	cv::Mat landmarkVertices;		// landmark point X, Y, Z coordinates in the cylindrical space (1xn)
	cv::Mat landmarkFaces;			// landmark triangle vertex indices (nx3)
	std::vector<std::vector<int>> landmarkMeshIndices;	// indices to mesh vertices in each landmark triangle
	cv::Mat meshVerts;			// cylindrical theta, phi, coordinates of mesh vertices (1xn)
	cv::Mat meshDepth;		// mesh vertex depth offset values from the landmark triangle surface (1xn)
	cv::Mat meshUV;			// UV coordinates of mesh vertices (1xn)
	cv::Mat meshFaces;		// XYZ and UV indices of mesh faces (nx6)
	cv::Mat faceRectIndices;		// Vector of 4 indices that defines the frontal face rect. 0: TOP 1: LEFT 2: BOTTOM 3: RIGHT (1x4)
	cv::Mat transformMatrix;		// mesh3d model transformation matrix to the template space(4x4)

	PolyMesh mesh3d;		// 3d template (optional)

	cv::Ptr<FaceModelTemplate> originalTemplate;	// a shared ptr to the original model template (only needed for partial template)

protected:

	// read a template yml file and an optional template obj file
	// call this only if the template type is already known. otherwise, call create() instead
	virtual bool read(const cv::String& tempYmlFile, const cv::String& tempObjFile = cv::String());

	//// load default TEMP_YML_FILE and TEMP_3D_FILE from a template dir
	//virtual bool loadDir(const cv::String& tempDir);

	// compute an alpha map in the cylindrical space based on facial features
	// alphaType defines the type of alpha map to return
	// e.g. ALP_EYE means only the eye region will have non zero alphas
	// mapSize controls the size of the alphaMap
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	virtual void computeAlphaMap(BlendType blendType, cv::Mat& alphaMap, cv::Rect& regionRect, cv::Size mapSize,
		int featherSize) const;


	// get reverse indices from meshVerts to landmarkFaces
	// the returned vector should have the same length as meshVerts
	// each item is the landmark face index the vertex belongs to
	virtual void getMeshToFaceIndices(std::vector<int>& faceIdx) const;

	//// return vertices inside all landmark triangles in cylindrical space coordinates
	//// sx and sy are scale for X and Y coords
	//// if baseShape is true, vetex coordinates are confined to its parent triangle
	//// If the template is not a complete template, originalTemplate must be set prior to calling this function
	//void getLMFaceVertices(
	//	float sx, float sy, std::vector<cv::Point3f>& faceVertices, bool baseShape=false) const;

};

} // namespace b3di
