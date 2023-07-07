/*M///////////////////////////////////////////////////////////////////////////////////////
//
// FrameStore is a container for captured frames (IR or RGB) with time stamps. It manages file I/O and memory caching.
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 1/31/2017	sec	created
//
//M*/
// 
#pragma once

#include <memory>
#include "FrameInfo.h"
#include "ImageContainer.h"

namespace b3di {

/** FrameStore is a container for captured frames (IR or RGB) with time stamps. It manages file I/O and memory caching. */
class FrameStore: public ImageContainer {
public:

	/**
	* FrameStore constructor
	* @param [in] cacheSize Max. number of frames to cache. Set it to 0 to cache all images.
	*/
	FrameStore();

	virtual ~FrameStore();


	/**
	* Construct an output file name from the prefix, fileIndex, and suffix
	*/
	virtual cv::String getOutputFileName(int fileIndex) const;

	/*
	* Get the input file name. Override base class method
	*/
	virtual cv::String getInputFileName(int fileIndex);

	/**
	* Add a frame to the frame store. frameImage will not be copied.
	* The frame will be cached in the frame store depending on the caching state
	* The frame may also be written to the disk if setImageWriting is true
	* 
	* @param [in] frameInfo
	* @param [in] frameImage
    * @return true if the adding is successful.
	*
	*/
	virtual bool addFrame(const FrameInfo& frameInfo, const cv::Mat& frameImage);

    /**
    * Add a frame to the frame store. frameImage will not be copied.
    * The frame will be cached in the frame store depending on the caching state
    * The frame may also be written to the disk if setImageWriting is true
    *
    * @param [in] frameInfo
    * @param [in] frameImage
    * @return true if the adding is successful.
    *
    */
    virtual bool addFrame(const FrameInfo& frameInfo, const b3di::TImage& frameImage);

	/**
	* Read a frame from file and store in the frame store
	* fileIndex is the index of the file in the input directory
	*
	* @param [in] input file index
	*
	*/
	virtual bool readFrame(int fileIndex);

	/**
	* Check if a frame has been added to the frame store. The frame may or may not be stored in the cache
	* 
	* @param [in] frameIndex Index of the frame to check
	* @param [out] frameInfo Frame info
	* @return True if the frame is in the frame store
	*/
	virtual bool hasFrame(int frameIndex, FrameInfo& frameInfo);

	/**
	* Get a frame from the frame store cache and also return its frameInfo record
	* Return an empty image if the frame is not in the cache.
	* 
	* @param [in] frameIndex Index of the frame to get
	* @param [out] frameInfo Frame info
	* @param [out] frameImage Frame image
	* @return True if the frame is in the cache
	*/
	virtual bool getFrameFromCache(int frameIndex, FrameInfo& frameInfo, cv::Mat& frameImage);

	/**
	* Replace the frame in the frame store.
	*
	* @param [in] frameIndex Index of the frame to set
	* @param [out] frameInfo Frame info
	* @param [out] frameImage Frame image
	* @return True if the frame is set
	*/
	virtual bool setFrame(int frameIndex, const FrameInfo& frameInfo, const cv::Mat& frameImage);


	virtual FrameInfo getFrameInfo(int frameIndex);

	virtual bool setFrameInfo(int frameIndex, const FrameInfo& frameInfo);

	/**
	* Write a frame from the frame store cache to the disk
	*
	* @param [in] frameIndex Index of the frame to write
	* @return True if the frame is written successfully 
	*/
	virtual bool writeFrame(int frameIndex);

	/**
	* Remove a frame from the frame store cache and return it
	* Return an empty image if the frame is not in the cache.
	*
	* @param [in] frameIndex Index of the frame to remove
	* @return Frame image removed the cache
	*/
	virtual cv::Mat removeFrameFromCache(int frameIndex);

	/**
	* Get last cached frame from frame store
	*
	* @param [out] frameIndex Index of the frame returned
	* @param [out] frameInfo Frame info
	* @param [out] frameImage Frame image
	* @return True if the frame is available
	*/
	virtual bool getLastCachedFrame(int& frameIndex, FrameInfo& frameInfo, cv::Mat& frameImage);

	/**
	* Shrink the frame store size.
	*
	* @param [in] newSize new frame store size. Ignore if it is larger than the current size.
	*/
	virtual void shrink(int newSize);

	virtual void clear();

	/* load input file names and update _frameInfo from the file names*/
	virtual void loadInputFileNames();


private:

	//void setFrameInfo(int frameIndex, const FrameInfo& frameInfo);

	std::vector<FrameInfo> _frameInfo;

	//int _frameCounter;	
};

typedef std::shared_ptr<FrameStore> FrameStorePtr;


} // namespace b3di