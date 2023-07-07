
#pragma once


#include <vector>
#include <opencv2/core/cvstd.hpp>
//#include "TDefs.h"

namespace b3di {


// Base class for all file container classes
// You should not be instantiating an object from this class directly
class FileContainer {
public:

    virtual ~FileContainer() {}

	// Set the file directory path to read/write files in the container
	virtual void setDirPath(const cv::String& dirPath);
	virtual cv::String getDirPath() const;

	// Set/get the file name prefix
	virtual void setFilePrefix(const cv::String& prefix);
	virtual cv::String getFilePrefix() const;

	// Set/get the file name suffix
	virtual void setFileSuffix(const cv::String& suffix);
	virtual cv::String getFileSuffix() const;

	// Set the range of files to read/write in the directory
	// For reading all files in a directory, set fromIndex to -1 (default)
	// For writing files, default from index is 1
	virtual void setFromIndex(int index);
	virtual int getFromIndex() const;
	virtual void setToIndex(int index);
	virtual int getToIndex() const;

	// Set the path to a flie that contains a list of file names to load
	virtual void setTocFile(const cv::String& tocPath) { _tocPath = tocPath; }

	// Traverse dirPath to get file names matching the prefix, suffix, from, to patterns
	// called automatically by readFile the first time so you shouldn't need to call this explicitly
	virtual void loadInputFileNames();

	// Define output file names to be used by writeFile
	// fileNames must have at least the same number of items as the output data size
	virtual void setOutputFileNames(const std::vector<cv::String>& fileNames) { _outFileNames = fileNames; }

	// loadFileNames must have been called first for these functions to work
	// Get all the file names
	std::vector<cv::String>& getInputFileNames();  // need to update the filenames inside the container
	virtual void getInputFileNames(std::vector<cv::String>& fileNames);

	// Get the file name for fileIndex
	virtual cv::String getInputFileName(int fileIndex);
	// Get the number of files
	virtual int getInputFileCount();

	// Construct an output file name from the prefix, fileIndex, and suffix
	virtual cv::String getOutputFileName(int fileIndex) const;

	// parse fileName to get time stamp
//#ifdef WIN32
	static int getTimeStampFromFileName(const cv::String& fileName);
//#endif
//#ifdef ANDROID  // why do we need this??? getTimeStampFromFileName works on Android
//	int getTimeStampFromFileName(const cv::String& fileName);
//#endif

	// These should be overriden by the dervied classes
	// read a file from a directory
	// loadFileNames is automatically called if it has not been called before
	virtual bool readFile(int fileIndex) = 0;
	// write a file to a directory
	virtual bool writeFile(int fileIndex)= 0;
	// Clear all data in the container
	virtual void clear() = 0;
	// Write out all items to files
	virtual bool writeFiles() = 0;
	// Return the number of files to read or the number of items in the container, whichever is greater
	virtual int getCount() = 0;

	// read all files
	virtual bool readFiles();

protected:

	// Make the constructor protected so you can't instantiate an object of this class
	FileContainer();

	// directory path for the files in the container
	cv::String _dirPath;

	cv::String _filePrefix;
	cv::String _fileSuffix;

	int _fromIndex, _toIndex;

	cv::String _tocPath;

	// a list of input file names
	std::vector<cv::String> _fileNames;
	std::vector<cv::String> _outFileNames;
};

} // namespace b3di