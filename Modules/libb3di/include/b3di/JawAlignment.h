#pragma once

#include <opencv2/core.hpp>
#include "HeadMeshI.h"

namespace b3di {

// Manual alignment adjustment between a head mesh and a jaw model by trying different rotations and translations
// to register a projected image of a jaw model with the teeth area of the frontal photo of the head model
// When the projected jaw images and the mouth image match based on user inspection, 
// the rotation and translation parameters are returned
// as an adjustment of transformation to be applied to the head model
class JawAlignment
{
public:

	enum MouthImageType {
		MOUTH_FRONTAL = 0,
		MOUTH_PROFILE = 1
	};

    JawAlignment();
    virtual ~JawAlignment() {};

    // Start a new alignment session. This function must be called first before other functions
    // head model is a teeth smile model
    // jawFile is a scan of the upper jaw to align with. jawCoordSpace is the upper jaw's input coordinate space.
    // The jawFile by default is the upper jaw file (recommended). 
    // If upper jaw scan is not available, you can use the lower jaw and should set "upperJaw" to false
    // If useInitialAlignment is true, the current headModel transformation will be used as the starting alignment with the jaw model.
    // Otherwise, it will recompute the initial alignment.
    virtual bool newAlignment(const HeadMeshI& headModel, const cv::String& jawFile, HeadMeshI::CoordSpace jawCoordSpace, 
        bool upperJaw=true, bool useInitialAlignment = true);

    // Same as the above but instead of jawCoordSpace, takes an input transform of the jaw model that will transform it to the default
    // CS_RIGHT_Y_UP coordinate space
    // This allows an arbitray input jaw transformation if it is not in one of the known CoordSpaces
    virtual bool newAlignment(const HeadMeshI& headModel, const cv::String& jawFile, const trimesh::xform& jawCoordXf, 
        bool upperJaw = true, bool useInitialAlignment = true);

    // A photo of the mouth area from the head model capturing session. This is the target of the alignment and does not change during the alignment.
    virtual cv::Mat getMouthPict(MouthImageType imageType= MOUTH_FRONTAL) const;

    // Return the rect of the mouth pict in the head model's frontal image space
    virtual cv::Rect getMouthRect(MouthImageType imageType = MOUTH_FRONTAL) const;

    // Return a mask image for the mouth pict where inside the mouth pixels have non-zero values
    //virtual cv::Mat getMouthMask() const;

    // Apply a rotation and translation transformation to the jaw model to try to match the target mouth pict. This will create a new jaw pict.
    // The translation unit is mm
    // The rotation unit is degree
    virtual bool tryTransformation(const cv::Point3f& trans, const cv::Vec3f& rot);

    // Get the current jaw pict after calling tryTransformation
    virtual cv::Mat getJawPict(MouthImageType imageType = MOUTH_FRONTAL) const;

    // Return a mask image for the jaw pict
    //virtual cv::Mat getJawMask() const;

    // Get the adjustment transformation based on the current rotation and translation applied in tryTransformation
    // The adjustment transfomation should be applied to the head model's current transformation
    // to align the head model with the jaw model
    virtual trimesh::xform getAdjustmentXform() const;

    // Use BGR channels to overlay the mouth and jaw picts so we can inspect their registration
    // The jaw pict has been transformed with the current transformation
    // The mouth pict stays the same in the session
    // B channel is from jawPict
    // R channel is from mouthPict
    // G channel is the average of the two picts
    // alpha controls the blending of the two picts. 0.0 means 100% mouthPict and 1.0 means 100% jawPict
    virtual cv::Mat getOverlayPict(double alpha=0.5, MouthImageType imageType = MOUTH_FRONTAL) const;

	// export the current jaw alignment with a frontal photo background in MeshLab MLP file for external photo alignment
	// including the camera information and the model transformation in the MLP file to render the jaw on top of the photo background
	// outputFMPFile must have a .mlp file extension. the ouptut file should be in the same dir as the jaw files
	// frontalPhotoFile should be a .jpg file and should be in the same dir as outputMLPFile.
	virtual bool exportAlignment(const cv::String& jawFile, const cv::String& outputYmlFile,
		const cv::String& outputPhotoFile, const cv::String& outputMlpFile) const;

    // render upper and lower jaw models to the frontal image using the same alignment
    //virtual cv::Mat renderJawToFrontalPict() const;

    // Call this function to end the alignment session. 
    // It will also free up memory used for the alignment.
    virtual bool endAlignment();


protected:

    bool transformJaw(const trimesh::xform& jawXf);
    //double computeRegistrationError() const;

private:
    PointCloud _jawModel;
    cv::Point3f _jawCenter;
    //cv::Point3f _mouthCenter;
    cv::Rect _mouthRects[2];
    //cv::Mat _mouthPicts[2];
	cv::Mat _facePicts[2];
	cv::Mat _jawPicts[2];
    //cv::Mat _mouthDepth;
    //cv::Mat _jawDepth;
    //cv::Mat _hitMask;
    //cv::Mat _mouthMask;
    //cv::Mat _jawMask;
    //cv::Mat _frontalPict;
    //cv::Rect _hitRect;
    CameraParams _imageCams[2];
    trimesh::xform _initialXf;
    trimesh::xform _headXf;
    trimesh::xform _jawCoordXf;
    trimesh::xform _lastXf;
    trimesh::xform _currentJawXf;
    trimesh::xform _jawToCamXfs[2];

};

} // namespace b3di

