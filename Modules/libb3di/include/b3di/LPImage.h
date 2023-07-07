#pragma once

#include <opencv2/core.hpp>

namespace b3di {


// A Laplacian Pyramid image 
class LPImage
{
public:
	LPImage();

    // Construct a LP image from an image
    // image can be CV_8UC3 (RGB), CV_8UC1 (gray) or CV_16UC1 (Depth).
    // maxLevel is the maximum number of levels in the pyramid (0 means subdivide until the imgage width/height is an odd number)
    // The subdivision stops when the image's width or height is not an even number 
	// or the image size (width or height) is smaller than baseBandImageSize
	// wrapAround is ignored for non-depth image
	LPImage(const cv::Mat& image, int maxLevel = 0, bool wrapAround = false, int baseBandImageSize = 8) {
		create(image, maxLevel, wrapAround, baseBandImageSize);
	}

	virtual ~LPImage();

	void create(const cv::Mat& image, int maxLevel = 0, bool wrapAround = false, int baseBandImageSize = 32);

    // Return the number of levels in the pyramid
    int getNumberOfLevels() const;

	// Reconstruct an image up to maxLevel levels
	// Reconstructed image has the same type of the input image
	// If maxLevel is less than the number of levels in the pyramid, resconstruction will stop at maxLevel
	// If maxLevel is 0, reconstruct from all levels, which should yield the same input image for non-depth image
	// For depth image, if diffThreshold >0, it will remove depth difference greater diffThreshold.
	// Also for depth image only, numSmoothingLevels control how many levels of the top diff images to smooth (e.g. 1 will smooth the final diff image)
    void reconstructImage(cv::Mat& outputImage, int maxLevel=0, cv::Scalar diffThreshold=cv::Scalar(0), int numSmoothingLevels=0) const;

    // Return an image band at certain level (highpass to lowpass)
    // Level 0 has the same resolution as the input image
    // If level is greater than the number of levels in the pyramid, the highest level image band is returned
    cv::Mat getImageBand(int level) const;

    // Replace a band image at a level
	// Level 0 is the difference image the same resolution as the input image (highest frequency)
	// level must be less than the number of levels in the pyramid
    // the input image must match the size of the band image to replace
    void replaceImageBand(int level, const cv::Mat& bandImage);

	// blend this LPImage to target using blendMask
	// All images, including blendMask, must be the same size and have the same number of levels
	// target image contains the blending result
	void blendTo(LPImage& target, const cv::Mat& blendMask, cv::Size filterSize) const;

	// Clone this LPImage to target
	void copyTo(LPImage& target) const;

	// Check if this is empty
	bool empty() const;

	// Clear all storage
	void clear();

private:

    bool _wrapAround;
	std::vector<cv::Mat> _imageBands;		// images from highpass to lowpass bands

};

} // namespace b3di

