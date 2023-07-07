#pragma once

#include <opencv2/core.hpp>

#include "FileContainer.h"
#include "PointCloud.h"


namespace b3di {

static const cv::String POINTCLOUD_PREFIX = "depth_M_";
static const cv::String POINTCLOUD_SUFFIX = ".ply";

// Container for camera files
class PointCloudContainer : public FileContainer {
public:

	PointCloudContainer() { setFilePattern(); }
    virtual ~PointCloudContainer() {}

	virtual void setFilePattern(const cv::String& filePrefix = POINTCLOUD_PREFIX, const cv::String& fileSuffix = POINTCLOUD_SUFFIX) {
        _filePrefix = filePrefix;
        _fileSuffix = fileSuffix;
    }

	// These must be called first before reading a depth (png) file
	void setCameraParams(const CameraParams& cam) { _camParams = cam; }
	// Set the range of depth points to keep when reading a depth image file; Use 0 for no limit. Not applicable for other files.
	//void setDepthRange(ushort minZ, ushort maxZ) { _minZ = minZ; _maxZ = maxZ; }
	//void setDepthRect(const Rect& bounds) { _depthRect=bounds; }

	// read a file from a directory if it has not been loaded previously
	virtual bool readFile(int fileIndex) {
		// Don't read it again if the data have been loaded
		if ((int)_pointclouds.size()>fileIndex && !_pointclouds[fileIndex].empty())
			return true;
		if (_fileNames.size() == 0)
			loadInputFileNames();
		_pointclouds.resize(_fileNames.size());
		cv::String filePath = getDirPath() + getInputFileName(fileIndex);
        if (filePath.empty())
            return false;
//        return _pointclouds[fileIndex].readPlyFile(filePath);
		return _pointclouds[fileIndex].readFile(filePath, _camParams);
	}

    // write a file to a directory
    virtual bool writeFile(int fileIndex) {
        if (fileIndex >= (int)_pointclouds.size())
            return false;
		cv::String outputFile = getDirPath() + getOutputFileName(fileIndex);

        return true;
// return _pointclouds[fileIndex].writePlyFile(outputFile);
    }

	virtual bool writeFiles() {

		for (int i = 0, n = (int)_pointclouds.size(); i < n; i++)
			if (!writeFile(i))
				return false;

		return true;
	}

    // return the number of point clouds
    //int getPointCloudsCount() const { return (int)_pointclouds.size(); }

	// Don't use getData because it will copy the point cloud
	// Use readFile(i) and then allData()[i] to access it
    // read from a file if necessary
    //bool getData(int index, PointCloud& data) {
    //    _pointclouds.resize(_fileNames.size());
    //    if (index >= (int)_pointclouds.size()) return false;

    //    if (_pointclouds[index].empty()) {
    //        readFile(index);
    //    }

    //    data = _pointclouds[index];

    //    return !data.empty();
    //}

	const b3di::PointCloud& getData(int index) {
		readFile(index);
		return _pointclouds[index];
	}
	void addData(const b3di::PointCloud& data) {
		_pointclouds.push_back(data);
	}

	std::vector<b3di::PointCloud>& allData() {
        return _pointclouds;
    }

	virtual void clear() {
		_pointclouds.clear();
		_fileNames.clear();
	}

	virtual int getCount() {
		if (_pointclouds.size() == 0)
			return (int)getInputFileCount();
		else
			return (int)_pointclouds.size();
//		return std::max((int)getInputFileCount(), (int)_pointclouds.size());
	}

private:

	std::vector<b3di::PointCloud> _pointclouds;
	b3di::CameraParams _camParams;				// needed for converting depth images to point cloud in readFile				

};

} // namespace b3di