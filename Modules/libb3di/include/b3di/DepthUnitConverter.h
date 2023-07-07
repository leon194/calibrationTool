#pragma once

#include <memory>
#include "platform.h"
#include "B3d_defs.h"
#include <opencv2/core.hpp>

namespace b3di {



class DepthUnitConverter
{
public:

	// to be used with setUnit for iPhone X quadratic coefficients
	static const double IPX_UNITS[3];

	DepthUnitConverter() {
		//setUnit(0, 0, 0);	
		setUnit(0.02, 0);	// default to linear conversion
	}

	// Assign coefficients for linear conversion
	void setUnit(double depthToZScale, double depthToZOffset = 0) {
		_depthToZScale = depthToZScale;
		_depthToZOffset = depthToZOffset;
	}

	// Assign coefficients for quadratic depth to z conversion
	void setUnit(double a, double b, double c) {
		// set quadratic function coefficients
		_a = a;
		_b = b;
		_c = c;

		// FIXME: need to compute a linear approximation at 30-50cm and set the coefficients
	}

	double getUnitScale() const {
		return _depthToZScale;
	}

	double getUnitOffset() const {
		return _depthToZOffset;
	}

	// Convert between z values (in mm) and depth values (0-65535)
	inline double depthToZ(double depthVal) const {

		// if quadratic coefficients are set, use quad conversion
		// FIXME: probably should use a lookup table
		if (_a != 0) {
			return _a*depthVal*depthVal + _b*depthVal + _c;
		}
		else {
			// use linear function
			return _depthToZScale*depthVal + _depthToZOffset;
		}
	}
	inline double zToDepth(double zVal) const {

		// FIXME: use linear to approximate quadratic for now 
		// as the inverse of quad requires a sqrt and is  expensive
		return (zVal- _depthToZOffset) / _depthToZScale;
	}
    inline double maxZ() const { /*return depthToZ(USHRT_MAX);*/ return MAX_FOREGROUND_Z; }
	inline unsigned short minDepth() const { return cv::saturate_cast<unsigned short>(zToDepth(MIN_FOREGROUND_Z)); }
	inline unsigned short maxDepth() const { return cv::saturate_cast<unsigned short>(zToDepth(MAX_FOREGROUND_Z)); }

private:
	// quadratic coefficients
	double _a, _b, _c;

	// linear coefficients
	double _depthToZScale;
	double _depthToZOffset;
};

// Set default depth unit converter
extern DepthUnitConverter g_depthUnitConverter;

} // namespace b3di
