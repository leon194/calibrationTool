#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include "FaceTracker.h"

namespace b3di {

class DepthSample {
public:
	// Construct a depth sample. count is the number samples of this value
	// subX and subY are sub pixel position (0-1) of the sample
	DepthSample(float depthVal, uchar w = 1/*, float subX = 0, float subY = 0*/);
	//DepthSample(float depthVal, uchar w = 1);
	bool operator<(const DepthSample& a) const
	{
		return this->value < a.value;
	}

	float value;	// sample value
	uchar weight;	// sample weight
	//uchar offset;	// sample sub-pixel position (0-4 for 2x2 sub pixels)
};



class TemporalVector
{
public:
	TemporalVector() {};
	
	void addSample(const DepthSample& sample);

	// Perform filtering of the sample vector and return the mean value of the samples within diffThreshold
	// minSamples is the min number of samples needed to be within diffThreshold to return a valid value
	// sample vector will be sorted first
	// If the vector has more than 1 layer, return the inner layer mean value, and 
	// use outerLayerVal to return the addtional outer layer value
	// sampleCount is the number of iniler samples
	// maxSampleWeight is the max weight of the inlier samples
	float filterVector(float diffThreshold, int& sampleCount, int& maxSampleWeight,
		float& maxDiff, int minSamples = 2, bool topLayer = false);

	const std::vector<DepthSample>& getSamples() const { return _samples; }

private:
	std::vector<DepthSample> _samples;

};


// A Temporal Buffer is used to accumulate temporal depth samples of varying lengths per pixel for temporal filtering
// The max number of samples per pixel is 255
class TemporalBuffer
{
public:

	// vector of temporal samples for a pixel
	struct SampleVector {
		std::vector<DepthSample> vals;
	};

	TemporalBuffer() { _bufferSize = cv::Size(); }

	virtual ~TemporalBuffer() {}

	// create and initialize a temporal buffer of with and height defined by bufferSize
	virtual void create(cv::Size bufferSize);

	// image must be single channel depth image of type CV_16UC1 or CV_32FC1
	virtual bool addImageToBuffer(const cv::Mat& depthImage);

    // add 3d points to buffer
    virtual bool addPointsToBuffer(const std::vector<cv::Point3f>& cylPoints, const std::vector<uchar>& pointWeights);

	// filter the buffer to return a depth image
	// confidenceMap is 8-bit confidence value of those samples in each pixel. 255 is max confidence.
	// depthThreshold is the depth difference threshold for merging multiple samples to a single value 
	// minNumSamples is the min number of samples within depthThreshold required to generate a filtered value
    virtual bool filterBuffer(cv::Mat& filteredDepthImage, cv::Mat& confidenceMap, 
		const FaceTracker::FaceLandmarks& faceLandmarks, float depthThreshold, int minNumSamples=2);

	// access sample vector of a pixel
	virtual const TemporalVector& getPixelVector(int x, int y) const {
		int index = y*_bufferSize.width + x;
		return _sampleBuffer[index];
	}

	virtual cv::Size getBufferSize() const { return _bufferSize; }

	// Check if this is empty
	virtual bool empty() const {
		return _bufferSize == cv::Size();
	}

	// Clear all storage
	virtual void clear() {
		_bufferSize = cv::Size();
		_sampleBuffer.clear();
	}


private:

	cv::Size _bufferSize;
	std::vector<TemporalVector> _sampleBuffer;		// store pixel vectors. size of the buffer should match _bufferSize
	//cv::Mat _sumMap;		// keep a running sum of the pixel vector values
};

} // namespace b3di

