#pragma once

#include <vector>
#include "platform.h"

// FileUtils reorganized by Jing Liu on March 2, 2017
// Before first Android APP release

// FIXME: don't need to cv::String, as long as it's not passed back to JNI
// JNI cannot handle empty std::string, but using internally should be fine
#include <opencv2/core.hpp>

namespace b3di {

class FileUtils {
public:
    
    // Append dir2 to dir1
    static cv::String appendPath(const cv::String& dir1, const cv::String dir2);

    // Cannot recursively create folders?
    static bool createDir(const cv::String& dir);
    
    static bool directoryExists(const cv::String& dir);

	// Check if a file exists; only implemented for WIN_32 now
	static bool fileExists(const cv::String& filePath);

    // Can't find reference using this function
    static cv::String getWorkingDir();


    //  --------  Added delete functions for cleaning up raw images, session directories  --------  //
    
    // return a list of full file paths (including dirPath) under a directory
    // TODO: Make sure it preserves alphabetical order
	static void getDirFiles(const cv::String& dirPath, std::vector<cv::String>& filePaths);

    // return a list of file names, matching certain search pattern
	// returned fileNames do no include the parent directory path
    // TODO: Make sure it preserves alphabetical order
    static void getDirFiles2(const cv::String& searchPattern, std::vector<cv::String>& fileNames);

    // delete a single file, return false if file doesn't exist or fail to delete
    static bool deleteFile(const cv::String& filePath);

    // delete files (NOT folders) inside the directory
    // will delete the folder "dir" as well
    // this function is *NOT* deleting files or folders iteratively
    static bool deleteDirFiles(const cv::String& dir);

    // Separate file and folder deletion, as recursive deletion could be very dangerous
    static bool deleteEmptyFolder(const cv::String& folderPath);

    // will delete everything inside "dir" (including the "dir" folder)
    //static bool deleteDirRecursively(const cv::String& dir);
    
    // return a list of folder names under searchPath
    static void getDirDirectories(const cv::String& searchPath, std::vector<cv::String>& folderNames);

	// Copy srcFile to dstFile
	static bool copyFile(const cv::String& srcFile, const cv::String& dstFile);

	static cv::String getBaseDir(const cv::String &filepath) {
		if (filepath.find_last_of("/\\") != cv::String::npos)
			return filepath.substr(0, filepath.find_last_of("/\\"));
		return "";
	}

	// https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path
	static cv::String getBaseFilename(const cv::String &filepath) {
		return filepath.substr(filepath.find_last_of("/\\") + 1);
	}

	// Return the file extension including the .
	static cv::String getFileExtension(const cv::String &filepath) {

		if (filepath.find_last_of(".") != cv::String::npos)
			return filepath.substr(filepath.find_last_of("."));
		return "";
	}

	// Return the file name without the extension
	static cv::String getFileNameNoExt(const cv::String& fileName) {
		cv::String::size_type pos = fileName.rfind('.', fileName.length());
		if (pos != cv::String::npos) {
			return fileName.substr(0, pos);
		}
		else {
			return fileName;
		}
	}

	// Read file and return binary data as a vector of uchar
	static bool readFileData(const cv::String& filePath, std::vector<uchar>& fileData);

	// Write binary data to file
	static bool writeFileData(const cv::String& filePath, const std::vector<uchar>& fileData);

};

} // namespace b3di