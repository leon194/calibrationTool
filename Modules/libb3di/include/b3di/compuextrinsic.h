
#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "CameraContainer.h"
#include "ImageContainer.h"
#include "DepthProcessor.h"


struct CompuExtrinsic_Input {

	CompuExtrinsic_Input() {
		inputCams = NULL;
		camImages = NULL;
        wait = 0;
        show = false;
        verbose = false;

		detectorType = b3di::CameraParams::DET_SURF;			// DET_SURF or DET_FACE
		maxIterations = 1;		// max number of iterations to run for SURF detector (min. is 1) Ignored for DET_FACE

		imageScale = 1.0;		// image resize scale (smaller number is faster. Only 1.0 is supported now)

		// Only applicable image camImages containers are used
		maxSearchM = 10;			// max number of M images to search for matching C image (using timestamp)
		maxSearchC = 5;			// max number of C images to search for matching M image (using timestamp)
		depthCameraType = b3di::DepthCamType::DEPTHCAM_B3D3;
		//depthProcConfig = b3di::DepthProcessor::DepthProcessorConfig();

    };

	b3di::CameraParams* inputCams;				// input L/R/M/C 4 camera params
	std::vector<cv::Mat> inputImages;			// input L/R/M/C 4 images. if the vector has less than 4 images, camImages containers will be searched to find the matching images
	b3di::ImageContainer* camImages;				// 4 image containers to search for matching images (using time stamps from their file names)

	b3di::CameraParams::DetectorType detectorType;
	int maxSearchM;
	int maxSearchC;
	int maxIterations;
	double imageScale;

	b3di::DepthCamType depthCameraType;
	//b3di::DepthProcessor::DepthProcessorConfig depthProcConfig;

    int wait;
    bool show;
    bool verbose;
};

struct CompuExtrinsic_Output {

	CompuExtrinsic_Output() {};

	b3di::CameraParams camC_computed;
};


int compuExtrinsic(CompuExtrinsic_Input& input, CompuExtrinsic_Output& output);

