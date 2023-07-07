
#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include "XForm.h"

#include "CameraParams.h"
#include "B3d_defs.h"
#include "B3d_utils.h"
#include "DepthCamProps.h"
#include "DepthUnitConverter.h"

namespace b3di {

class DepthProcessor {
public:

	struct DepthProcessorOpts {

		DepthProcessorOpts() {

			//depthResolution = 0;
			//depthRanges[0] = (ushort)g_depthUnitConverter.minDepth();
			//depthRanges[1] = (ushort)g_depthUnitConverter.maxDepth();

			//minZ = MIN_FOREGROUND_Z;
			//maxZ = MAX_FOREGROUND_Z;

			show = false;
			verbose = false;
			//clipDepth = true;
		}

		//double depthResolution;				// in mm
		//ushort depthRanges[2];		// min/max depth ranges
		//double minZ, maxZ;
		bool show;
		bool verbose;
		//bool clipDepth;		// remove depth outside of the range
	};

	// TODO: deprecate this
	struct DepthProcessorConfig {

		DepthProcessorConfig() {
			// set default using B3D3 values
			stereoBaseline     = 50.0;
			//stereoMaxDisparity = 288;
			stereoBlockSize    = 19;
		}

		double stereoBaseline;
		//double stereoMaxDisparity;
		double stereoBlockSize;
	};

	DepthProcessor();
	virtual ~DepthProcessor();

	// This function is kept for backward compatibility but should not used anymore
	// Use setDethCameraType instead
	// TODO: deprecate this
	void setDepthProcessorConfig(const DepthProcessorConfig& config);


	// void setDepthCameraType(b3di::DepthCameraConfig::DeviceType depthCamType);
	void setDepthCameraType(b3di::DepthCamType depthCamType);

	// set camera params for left/right stereo camera
	void setStereoCameras(const CameraParams camParams[2], float frameScale=1.0f);

	// set the target camera for depth image computation
	void setTargetCamera(const CameraParams& camParam, float frameScale = 1.0f);

	void setOptions(const DepthProcessorOpts& opts);

	DepthProcessorOpts getOptions() const { return _opts; }

//	int computeDepth(const Mat stereoFrames[], Mat& depthImg, bool resetHeadRect = false, const Mat& targetFrame=Mat());
	// Compute depthImg from stereoFrames
	// If depthRect is set, limit the depth computation to only pixels inside depthRect
//	int computeDepth(b3di::DepthCamera::DeviceType depthCamType, const cv::Mat stereoFrames[], cv::Mat& depthImg, const cv::Rect& depthRect = cv::Rect()) const;
	int computeDepth(const cv::Mat stereoFrames[], cv::Mat& depthImg, const cv::Rect& depthRect = cv::Rect()) const;

private:
	DepthProcessorOpts _opts;

	float _stereoFrameScale;
	float _targetFrameScale;

	// TODO: deprecate this
	// obtained from DepthCamera
	float _stereoBaseline;
	//int _stereoMaxDisparity;	// not used
	int _steroBlockSize;


	b3di::DepthCamType _depthCamType;

//	Rect _headRect;
	CameraParams _camParams[3];

};

} // namespace b3di