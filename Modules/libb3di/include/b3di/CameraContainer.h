#pragma once

#include <memory>

#include "platform.h"
#include "FileContainer.h"
#include "CameraParams.h"

#include "TLog.h"
//using namespace std;
//using namespace cv;

namespace b3di {

static const cv::String CAMERA_PREFIX = "cam_";
static const cv::String CAMERA_SUFFIX = ".yml";

// Container for camera files
class CameraContainer: public FileContainer {
public:

	CameraContainer() { setFilePattern(); }
	virtual ~CameraContainer() {}

    // Package output files operations together
	virtual void initialize(const cv::String& dirPath, const cv::String& filePrefix = CAMERA_PREFIX, const cv::String& fileSuffix = CAMERA_SUFFIX)
    {
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  initialize");
        setDirPath(dirPath);
        setFilePattern(filePrefix, fileSuffix);
        loadInputFileNames();
    }

	virtual void finalize(const cv::String& dirPath, const cv::String& filePrefix = CAMERA_PREFIX, const cv::String& fileSuffix = CAMERA_SUFFIX)
    {
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  finalize");
        setDirPath(dirPath);
        setFilePattern(filePrefix, fileSuffix);
        writeFiles();
    }

	virtual void setFilePattern(const cv::String& filePrefix = CAMERA_PREFIX, const cv::String& fileSuffix = CAMERA_SUFFIX) {
		_filePrefix = filePrefix;
		_fileSuffix = fileSuffix;
	}

	// read a file from a directory if it has not been loaded previously
	virtual bool readFile(int fileIndex) {
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  readfile: %d", fileIndex);
		// Don't read it again if the data have been loaded
		if ((int)_cameras.size()>fileIndex && !_cameras[fileIndex].empty())
			return true;
		if (_fileNames.size() == 0)
			loadInputFileNames();
		if (_fileNames.size() < (fileIndex + 1))
			return false;
		cv::String filePath = getDirPath() + getInputFileName(fileIndex);
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  readfile: %s", filePath.c_str());
		if (filePath.empty())
			return false;
		if (_cameras.size() < _fileNames.size())
			_cameras.resize(_fileNames.size());
		return _cameras[fileIndex].loadParams(filePath);
	}

	// write a file to a directory
	virtual bool writeFile(int fileIndex) {
		if (fileIndex >= (int)_cameras.size())
			return false;
		cv::String tempName = getOutputFileName(fileIndex);
		cv::String tempDirPath = getDirPath();
		cv::String outputFile = tempDirPath + tempName;
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  writefile: %s", outputFile.c_str());

// What's this for??
//#ifdef ANDROID
//		_outFileNames.push_back(tempName);
//#endif
		return _cameras[fileIndex].writeParams(outputFile);
	}

	virtual bool writeFiles() {
//		TLog::log(TLog::LOG_VERBOSE, "JIA-------CameraContainer.h  writefiles camera file number = %d", (int)_cameras.size());
// What's this for??
//#ifdef ANDROID
//		_outFileNames.clear();
//#endif
		for (int i = 0, n = (int)_cameras.size(); i < n; i++)
			if (!writeFile(i))
				return false;

		return true;
	}


	// read from a file if necessary
	//bool getData(int index, CameraParams& data) {
	//	_cameras.resize(_fileNames.size());
	//	if (index >= (int)_cameras.size()) return false;
	//	if (_cameras[index].empty()) {
	//		readFile(index);
	//	}
	//	data = _cameras[index];
	//	return !data.empty();
	//}
	const b3di::CameraParams& getData(int index) {
		static CameraParams emptyCam;
		if (readFile(index))
			return _cameras[index];
		else
			return emptyCam;
	}
	void addData(const b3di::CameraParams& cam) {
		_cameras.push_back(cam);
	}

	std::vector<b3di::CameraParams>& allData() {
		return _cameras;
	}

	virtual void clear() {
		_cameras.clear();
		_fileNames.clear();
	}

	virtual int getCount() {
		if (_cameras.size() == 0)
			return (int)getInputFileCount();
		else
			return (int)_cameras.size();
		//return std::max((int)getInputFileCount(), (int)_cameras.size());
	}

private:

	std::vector<b3di::CameraParams> _cameras;

};
typedef std::shared_ptr<CameraContainer> CameraContainerPtr;

} // namespace b3di