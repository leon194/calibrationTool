
#pragma once
#include "platform.h"

#include <opencv2/core.hpp>
#include <stdio.h>
#include "XForm.h"


//using namespace trimesh;
//using namespace cv;

namespace b3di {


class CameraParams {
public:

//	enum { ROTATE_XYZ=0, ROTATE_ZYX=1 };

	// for computeExtrinsic
	enum DetectorType {
		DET_SURF,			// SURF detector
		DET_FACE,			// Face detector
	};

	enum ProjectionType {
		PROJ_PERSPECTIVE,			// perspective camera
		PROJ_CYLINDRICAL,			// cylindrical camera
		PROJ_CYLINDRICAL_DOME		// cylindrical dome camera
	};

	CameraParams();

	CameraParams(const CameraParams& a) { a.copyTo(*this); }

	// create a cylindrical projection camera
	// imageSize is the output cylindrical map size
	// centerOfProjection is the center of frontal face, which will be mapped to the center of the cylindrical projection
	// intrnsic is the intrinsic params of the camera use to project to depth images to the cylindrical map
	// faceRect is the image rect of the face in frontCam.
	// faceDistance is the distance to the face from the camera in mm
	// headPose is the subject head's rotation angles
	// bottomCropRatio controls the crop line below the bottom of the face rect and should be >1.0. The larger the number, the lower the crop line.
	// projTiltAngle is the head tilt angle (rad) to be applied when projected on the cylindrical map (e.g. rotate head back slightly to have better nostril)
	// projType should be either PROJ_CYLINDRICAL or PROJ_CYLINDRICAL_DOME
	void createCylindrical(const cv::Size& imageSize, cv::Point2f& centerOfProjection,
		const cv::Mat& intrnsic, const cv::Rect& faceRect, double faceDistance, const cv::Vec3d& headPose, 
		double bottomCropRatio=1.3, double projTiltAngle=0, ProjectionType projType= PROJ_CYLINDRICAL_DOME);

	double fx() const { return _fx; }
	double fy() const { return _fy; }
	double cx() const { return _cx; }
	double cy() const { return _cy; }

	cv::Size getImageSize() const { return _imageSize; }
	cv::Mat getDistCoeffs() const { return _distCoeffs; }
	bool hasDistCoeffs() const { return !_distCoeffs.empty() && _distCoeffs.at<double>(0) != 0; }

	// compute camera lookAt point and up vector from the position and rotation values
	cv::Vec3d getLookAtPoint(double distanceFromCam) const;
	cv::Vec3d getUpVector() const;
	cv::Vec3d getPosition() const;
	cv::Vec3d getRotation() const;

	// return a normalized rotation vector and the rotation angle theta (in radian)
	cv::Vec4d getRotationVector() const;

	cv::Mat getIntrinsicMat() const;

	cv::Mat getRMatrix() const;
	cv::Mat getTMatrix() const;

	cv::Mat getR1Matrix() const { return _R1;  }
	cv::Mat getQMatrix() const { return _Q; }

	void copyTo(CameraParams& dstParams) const;
	CameraParams& operator= (const CameraParams& a) {
		a.copyTo(*this);
		return *this;
	}

	void setIntrinsic(double fx, double fy, double cx, double cy, cv::Size imageSize) {
		_fx = fx; _fy = fy; _cx = cx; _cy = cy;
		_imageSize = imageSize;
	}

	void setDistCoeffs(const cv::Mat& distCoeffs) {
		_distCoeffs = distCoeffs.clone();
	}

	void setRMatrix(const cv::Mat& R_mat) {
		_R = R_mat.clone();
	};
	void setTMatrix(const cv::Mat& T_mat) {
		_T = T_mat.clone();
	};

	void setR1Matrix(const cv::Mat& R1_mat) {
		_R1 = R1_mat.clone();
	};
	void setQMatrix(const cv::Mat& Q_mat) {
		_Q = Q_mat.clone();
	};

	void setIdentity() {
		_R = cv::Mat::eye(3, 3, CV_64FC1);
		_T = cv::Mat::zeros(3, 1, CV_64FC1);
	}

	void setImageSize(cv::Size newSize);
	void scaleImageSize(float imageScale);

	// Change the intrinsic params based on the cropRect
	void setCroppingRect(const cv::Rect& cropRect);

	// return the rotational transformation matrix that can rotate a vector in the camera space to the world space
	// this doesn't include the translation
	//xform getRotTransform() const;

	// return the transformation matrix that takes a point in the world space to the camera space
	// this includes both rotation and translation
	// If reverseOrder is true, the coordinates will be translated first and then rotated
	// The default is rotation first and than translation
	trimesh::xform getWorldToCameraXform(bool reverseOrder = false) const;

	// return the transformation matrix that takes a point in the camera space to the world space
	// this is the inverse transformation of getWorldToCameraXform	
	trimesh::xform getCameraToWorldXform(bool reverseOrder = false) const;

	void setCameraToWorldXform(const trimesh::xform& xf_cw);

	void setWorldToCameraXform(const trimesh::xform& xf_wc);

	// rotationOrder should be ROTATE_XYZ or ROTATE_ZYX
	void setRotationOrder(int /* rotationOrder */) { /*_rotationOrder = rotationOrder;*/ }

	// load params from file
	bool loadParams(const cv::String& filePath);

	bool writeParams(const cv::String& filePath, bool encrypt=false) const;

	bool writeParams(cv::FileStorage& fs) const;
	bool loadParams(const cv::FileStorage& fs);

	bool empty() const {
		return _imageSize.width == 0;
	}

	// return true if the the extrinsic params are valid
	bool hasExtrinsic() const {
		return !_R.empty() && !_T.empty();
	}

	// invalidate the extrinsic params
	void invalidateExtrinsic() {
		_R = cv::Mat();
		_T = cv::Mat();
	}

    // This function is not supported on iOS and will return false
	// Compute the extrinsic params given srcImage, srcDepth, srcCam.
	// Match features between srcImage and targetImage to find transformation between src and target using solvePnP
	// detectorType is used to find features in the images for matching
	// surfDetThreshold should be between 200-400 (smaller number finds more feature points)
    bool computeExtrinsic(const cv::Mat& srcImage, const cv::Mat& srcDepth, const CameraParams& srcCam, const cv::Mat& targetImage,
		DetectorType detectorType = DET_SURF, float errThreshold = 1.5f, 
		cv::Vec2i xRanges = cv::Vec2i(), cv::Vec2i yRanges = cv::Vec2i(), bool show = false, int surfDetThreshold = 400,
		int minNumFeatures=6);

	// Convert point to/from image and camera space
	cv::Point3f imageToCameraSpacePoint(const cv::Point3f& imagePoint) const;
	cv::Point3f cameraToImageSpacePoint(const cv::Point3f& cameraSpacePoint) const;

	// Return true if the camera defines a cylindrical projection
	bool isCylindricalProjection() const { return _cx == 0.0; }

	ProjectionType getProjectionType() const;

	// Create an authentication code from the deviceId and camera params
	// The code will be stored internally and written to the file by writeParams
	void createAuthCode(const cv::String& deviceId);

	// Verify the authentication code loaded from a file is valid (matching the camera params and deviceId)
	// Call this after loadParams
	// Return true if the authentication code is present and valid
	bool verifyAuthCode(const cv::String& deviceId) const;

	// Set cylindrical projection params for PROJ_CYLINDRICAL_DOME
	// set the cylinder top and bottom cap offsets from the center of the cylinder (in mm)
	// Set them to 0 for PROJ_CYLINDRICAL (no top and bottom caps)
	void setCylProjection(double cylTopOffset = 0, double cylBottomOffset = 0) {
		_cylTopY = cylTopOffset;
		_cylBottomY = cylBottomOffset;
	}

	//// set baseHeight to non-zero for type PROJ_CYLINDRICAL_DOME
	//// set baseHeight to 0 to change projection type to PRO_CYLINDRICAL
	//void setDomeBaseHeight(double baseHeight) {
	//	_domeBaseHeight = baseHeight;
	//}
	void getCylProjection(double& topOffset, double& bottomOffset, double& topRow, double& bottomRow) const {
		topOffset = _cylTopY; bottomOffset = _cylBottomY;
        topRow = (_cylTopY > 0) ? (_cy - _cylTopY*_fy) : 0; if (topRow < 0) topRow = 0;
		bottomRow= (_cylBottomY>0) ? (_cy + _cylBottomY*_fy) : 0;
	}


private:

	// Return an encryption key using sha256 from deviceId and camera params
	cv::String encrypt(const cv::String& deviceId) const;

	double _fx, _fy, _cx, _cy;

	cv::Size _imageSize;
	cv::Mat _distCoeffs;

//	int _rotationOrder;	// always set to ROTATE_XYZ for now 
	cv::Mat _R, _T;		// Extrinsic rotation and translation matrices
	cv::Mat _Q, _R1;	// set only if used with a disparity map (see cv::stereoRectify)

	cv::String _authCode;	// authentication code is created by encrypt or loaded from file
	
	ProjectionType _projType;
	double _cylTopY;	// for PROJ_CYLINDRICAL_DOME only. distance to the top of the cylindrical projection from the center of the projection in mm
	double _cylBottomY;	// for PROJ_CYLINDRICAL_DOME only. distance to the bottom of the cylindrical projection from the center of the projection in mm

};

} // namespace b3di
