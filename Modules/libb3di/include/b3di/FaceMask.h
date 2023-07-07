#pragma once

#include <vector>
#include <opencv2/core.hpp>
#include "FaceTracker.h"
#include "HeadMeshI.h"

namespace b3di {

struct FaceMaskArgs {

	float width_arg;
	float height_arg;
	int smooth_arg;
	int smoothInner_arg;
	float offset_arg;
	float chin_arg;
	float chinY_arg;
	float chinZ_arg;
	float chinV_arg;

	float rotateX_arg;
	float rotateY_arg;
	float rotateZ_arg;

	float trenchWidth_arg;
	float trenchDepth_arg;

	float logoH_arg;
	float logoP_arg;

	cv::String textRight_arg;
	cv::String textLeft_arg;

	float noseWidth_arg;
	float noseOffset_arg;
	float noseTangentOffset_arg;

	cv::String textT_arg;

	cv::String hookFile_arg;

	cv::String contourType_arg;
	//int hookType_arg;

	cv::String gripFile_arg;
	float gripOffsetZ_arg;

	cv::String fullMaskFile_arg;
	cv::String fullMaskHook_arg;

	float widthOffset_arg;

	float offsetTangent_arg;

	cv::Mat logoImage, iconImage;

};

class FaceMask {
public:

	FaceMask();
	virtual ~FaceMask();

	// return true if the face mask contour data are empty
	bool empty() const;

	//  clear the contour data
	void clear();

	// load mask contour data from a yaml file
	bool loadContour(const cv::String& inputFile);

	// write mask contour data to a yaml file
	bool writeContour(const cv::String& outputFile) const;

	// create the mask contour from a headMesh
	bool createContour(b3di::HeadMeshI& headMesh);

	// return contour data (read-only)
	cv::String getContourType() const { return _contourType; }
	const std::vector<cv::Point3f>& getContourPoints() const { return _contourPoints; }
	const std::vector<cv::Point3f>& getContourNormals() const { return _contourNormals; }

	// create a face mask as a PolyMask from the contour data
	// use FaceMaskArgs to customize the mask
	// contour data must have been loaded or created
	bool createMask(const FaceMaskArgs& args, b3di::PolyMesh& pm);

	/*** Below are experimental functions. Don't use them yet ***/
	// compute avg PolyMesh from Google FaceMesh files
	// the avg PolyMesh has fixed orentation (front facing)
	// faceMeshDir is the path that contains the FaceMesh files
	bool computeAvgFaceMesh(const cv::String& faceMeshDir, b3di::PolyMesh& faceMesh);

	// create contour from Google FaceMesh PolyMesh
	// faceMesh is a PolyMesh created from Google FaceMesh
	bool createContourFromFaceMesh(const b3di::PolyMesh& faceMesh);

	//// create contour from Google FaceMesh files
	//// faceMeshDir is the path that contains the FaceMesh files
	//bool createContourFromFaceMesh(const cv::String& faceMeshDir);
	/*** Above are experimental functions. Don't use them yet ***/

private:

	cv::String _contourType;
	std::vector<cv::Point3f> _contourPoints;
	std::vector<cv::Point3f> _contourNormals;
	b3di::FaceTracker::FaceLandmarks _faceLms;

};

} // namespace b3di
