#pragma once

#include <opencv2/core.hpp>
#include "B3d_utils.h"

// https://razibdeb.wordpress.com/2013/09/10/skin-detection-in-c-using-opencv/
class SkinDetector
{
public:

	//YCrCb threshold for skin detection
	// Chai, Douglas, and King N. Ngan. “Face segmentation using skin-color map in videophone applications.” Circuits and Systems for Video Technology, IEEE Transactions on 9.4 (1999): 551-564.
	SkinDetector(int y_min = 15, int y_max = 240, int cr_min = 133, int cr_max = 173, int cb_min = 77, int cb_max = 127) {
		setRange(y_min, y_max, cr_min, cr_max, cb_min, cb_max);
	}
	~SkinDetector(void) {};

	void setRange(int y_min, int y_max, int cr_min, int cr_max, int cb_min, int cb_max) {
		Y_MIN = y_min;
		Y_MAX = y_max;
		Cr_MIN = cr_min;
		Cr_MAX = cr_max;
		Cb_MIN = cb_min;
		Cb_MAX = cb_max;
	}

	//Return a skin masked image (255 for skin and 0 for non-skin area)
	cv::Mat getSkinMask(const cv::Mat& input) const {
		cv::Mat skin;
		//first convert our RGB image to YCrCb
		cv::cvtColor(input, skin, cv::COLOR_BGR2YCrCb);

		//uncomment the following line to see the image in YCrCb Color Space
		//b3di::showImage("YCrCb Color Space",skin);

		//filter the image in YCrCb color space
		cv::Mat mask;
		cv::inRange(skin, cv::Scalar(Y_MIN, Cr_MIN, Cb_MIN), cv::Scalar(Y_MAX, Cr_MAX, Cb_MAX), mask);
		//b3di::showImage("Skin mask", mask);

		return mask;
	}




private:
	int Y_MIN;
	int Y_MAX;
	int Cr_MIN;
	int Cr_MAX;
	int Cb_MIN;
	int Cb_MAX;
};