#pragma once

#include <vector>
#include <opencv2/core.hpp>

#include "CameraContainer.h"
#include "FrameStore.h"

namespace b3di {

	// output selectedFrameIndices
	// Must match HeadMeshI::ProfileImageIndex
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

	int selectFirstFrame(b3di::FrameStorePtr frameStore, int maxFrames, b3di::CameraContainer& framePoseContainer);

	// select frames to be used for texture mapping
	// headPoseContainer contains the M cam frame pose
	// return selected fram indices
	// maxNumSelected controls the max number of selected frames
	// If maxSourceFrames>0, only choose from at most this number of M cam frames (0 to choose from all)
	// If skipFrame is 1, skips the odd number frames from the selection
	// If skipFrame is 2, skips the even number frames from the selection
	bool selectFrames(b3di::CameraContainer& headPoseContainer, b3di::FrameStorePtr mFrameStore,
		int scanMode, int frontalFrameIndex, cv::Vec3d frontHeadPose,
		std::vector<int>& selectedFrameIndices, int maxNumSelected=9,
		int maxSourceFrames=0, int skipFrame=0);


}  // End of namespace b3di

