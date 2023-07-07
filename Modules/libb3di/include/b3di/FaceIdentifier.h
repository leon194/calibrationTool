
#pragma once


#include <vector>
#include <opencv2/core.hpp>


#include "CameraParams.h"
#include "FaceTracker.h"
#include "PointCloud.h"
#include "DepthCamProps.h"

namespace b3di {

	struct FaceModelFileNames {
		cv::String infoFile, depthFile, cameraFile, textureFile, landmarkFile;
	};

	struct FaceIdResult {
		FaceIdResult() { error = -1; similarity = 0; }
		cv::String modelName;
		double error;		// Face identification error (0 means no error, -1 means match not performed or is invalid)
		double similarity;	// between 0 and 1 with 1 being identical
	};

	class FaceIdentifier {
	public:

		FaceIdentifier();
		virtual ~FaceIdentifier();

		/*
		* Set the model file names needed for FaceID
		*/
		void setModelFileNames(const FaceModelFileNames& modelFileNames);

		/*
		* Prepare input L/R stereo frames for identification
		* This function is designed for B3D cameras
		* Return false if input is invalid or spoofing is detected
		*/
		bool prepareSample(const cv::Mat inputImages[],
			const b3di::CameraParams inputCams[],
			b3di::DepthCamType depthCamType, bool detectSpoof=true, bool showIt = false);

		/*
		* Prepare sample image for identification in the input region faceRect
		* Use this function if depth image is available
		* Takes a sampleImage, camera parameters, depth image sampleDepth
		* Finds landmarks in the sample image
		* Return false if input is invalid or spoofing is detected
		*/
		bool prepareSample(const cv::Mat& sampleImage,
			const b3di::CameraParams& imageCam,
			const cv::Mat& sampleDepth, cv::Rect faceRect, bool showIt = false);

		/*
		* clears point cloud and releases cv::Mat containers used in prepareSample
		* should be called before calling prepareSample
		*/
		void clearSample();

		/*
		* Prepare a target model for veryfication with the sample
		* Load the depth image from modelDir and create a target face model
		* It will replace the previous target model
		*/
		bool prepareTarget(const cv::String& modelDir);

		/*
		* clear the target model and free up memory
		*/
		void clearTarget();

		/*
		* Verify the current sample with the current target model
		* Returns true if the model's similairity value is above similarityThresh
		* Also returns FaceIdResult for the model.
		*/
		bool verify(double similarityThresh, FaceIdResult& faceIdResult, bool showIt = false);

		/*
		* Same as verify() but will call prepareTarget automatically first.
		* It will replace the current target model too.
		* Kept for backward compatibility. New code should call prepareTarget explicitly
		*/
		bool verify(const cv::String& modelDir, double similarityThresh,
			FaceIdResult& faceIdResult, bool showIt = false);

	private:

		// Detect if the input image is a spoof (mask) 
		static bool detectSpoof(const cv::Mat irImage, b3di::DepthCamType depthCamType);

		// Compute face feature size from key landmarks
		static double computeFeatureSize(const FaceTracker::FaceLandmarks& features);

		// Compute similarity value from errors
		static double computeSimilarity(double icpError, double sizeError, double lmsError);

		// Match the face inside the faceRect of the sample images with the target face model
		// Returns true if it is a match
		// Also returns the error and confidence of the match result
		bool matchModel(PointCloud& samplePC,
//			const cv::Mat& sampleImage, 
			const b3di::CameraParams& sampleCam,
			const b3di::FaceTracker::FaceLandmarks& sampleFeatures,
			PointCloud& targetPC,
			const cv::Mat& modelTexture,
			const FaceTracker::FaceLandmarks& modelFeatures,
			double& error, double& similarity,
			bool showIt = false);

		FaceTracker::FaceLandmarks _sampleFeatures2;
//		cv::Mat _sampleImage2;
		CameraParams _sampleCam2;
		PointCloud _samplePC;
		//std::vector<cv::String> _modelDirs;
		FaceModelFileNames _modelFileNames;

		cv::String _targetName;
		PointCloud _targetPC;
		cv::Mat _targetTexture;
		FaceTracker::FaceLandmarks _targetFeatures;

	};

} // namespace b3di