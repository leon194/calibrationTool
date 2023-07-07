#pragma once

#include "FileContainer.h"
#include "FaceTracker.h"

#include <opencv2/core.hpp>

namespace b3di {

static const cv::String LANDMARK_PREFIX = "feature_";
static const cv::String LANDMARK_SUFFIX = ".yml";

// Container for face landmarks files
class FaceLandmarkContainer: public FileContainer {
public:

	FaceLandmarkContainer() { setFilePattern(); }
	virtual ~FaceLandmarkContainer() {}

	virtual void setFilePattern(const cv::String& filePrefix = LANDMARK_PREFIX, const cv::String& fileSuffix = LANDMARK_SUFFIX) {
		_filePrefix = filePrefix;
		_fileSuffix = fileSuffix;
	}

	// read a file from a directory if it has not been loaded previously
	virtual bool readFile(int fileIndex) {
		// Don't read it again if the data have been loaded
		if ((int)_landmarks.size()>fileIndex && !_landmarks[fileIndex].empty())
			return true;
		if (_fileNames.size() == 0)
			loadInputFileNames();
		_landmarks.resize(_fileNames.size());
		cv::String filePath = getDirPath() + getInputFileName(fileIndex);
		if (filePath.empty())
			return false;
		return FaceTracker::readFile(filePath, _landmarks[fileIndex]);
	}

	// write a file to a directory
	virtual bool writeFile(int fileIndex) {
		if (fileIndex >= (int)_landmarks.size())
			return false;
		cv::String outputFile = getDirPath() + getOutputFileName(fileIndex);
		return FaceTracker::writeFile(outputFile, _landmarks[fileIndex]);
	}

	virtual bool writeFiles() {

		for (int i = 0, n = (int)_landmarks.size(); i < n; i++)
			if (!writeFile(i))
				return false;

		return true;
	}


	// read from a file if necessary
	//bool getData(int index, FaceTracker::FaceLandmarks& data) {
	//	_landmarks.resize(_fileNames.size());
	//	if (index >= (int)_landmarks.size()) return false;
	//	if (_landmarks[index].empty()) {
	//		readFile(index);
	//	}
	//	data = _landmarks[index];
	//	return !data.empty();
	//}

	const FaceTracker::FaceLandmarks& getData(int index) {
		readFile(index);
		return _landmarks[index];
	}
	void addData(const FaceTracker::FaceLandmarks& data) {
		_landmarks.push_back(data);
	}

	std::vector<FaceTracker::FaceLandmarks>& allData() {
		return _landmarks;
	}

	virtual void clear() {
		_landmarks.clear();
		_fileNames.clear();
	}

	virtual int getCount() {
		return std::max((int)getInputFileCount(), (int)_landmarks.size());
	}

private:

	std::vector<FaceTracker::FaceLandmarks> _landmarks;

};

} // namespace b3di