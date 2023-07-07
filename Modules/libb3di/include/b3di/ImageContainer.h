
#pragma once
#include "platform.h"


#include "FileContainer.h"
#include "TImage.h"


namespace b3di {

//static const cv::String IMAGE_COLOR_PREFIX = "color_";
//static const cv::String IMAGE_DEPTH_PREFIX = "depth_";
//static const cv::String IMAGE_IR_PREFIX = "ir_";
//static const cv::String IMAGE_EYELASH_PREFIX = "eyelash_";

static const cv::String IMAGE_COLOR_PREFIX = "";
static const cv::String IMAGE_DEPTH_PREFIX = "";
static const cv::String IMAGE_IR_PREFIX = "";

static const cv::String IMAGE_PNG_SUFFIX = ".png";
static const cv::String IMAGE_JPG_SUFFIX = ".jpg";


// Container for all image types
class ImageContainer: public FileContainer {
public:

	enum ImageDataType
	{
		IMAGE_UNCHANGED = -1,
		IMAGE_RGB = 0,			// 24-bit rgb
		IMAGE_GRAY = 1,			// 8-bit gray scale
		IMAGE_DEPTH = 2,		// 16-bit depth
	};

	ImageContainer();

	virtual ~ImageContainer();

    // Package output files operations together
	virtual void initialize(const cv::String& dirPath, const cv::String& filePrefix = IMAGE_COLOR_PREFIX, const cv::String& fileSuffix = IMAGE_PNG_SUFFIX, ImageDataType dataType = IMAGE_RGB,
		bool imageCaching=false, int imageRotation=0, int outputQuality = -1);
//    virtual void finalize(const string& dirPath, const string& filePrefix = IMAGE_COLOR_PREFIX, const string& fileSuffix = IMAGE_PNG_SUFFIX, ImageDataType dataType = IMAGE_RGB);


	virtual void setFilePattern(const cv::String& filePrefix = IMAGE_COLOR_PREFIX, const cv::String& fileSuffix = IMAGE_PNG_SUFFIX, ImageDataType dataType = IMAGE_RGB, int outputQuality = -1);

	// read a file from a directory if it has not been loaded previously
	// DON'T call this directly. Use getImage instead
	// caching must be enabled
	virtual bool readFile(int fileIndex);

	// write a image in _images to a directory
	// caching must be enabled
	virtual bool writeFile(int fileIndex);

	virtual bool writeFiles();

	virtual void copyFrom(const ImageContainer& src, bool copyCache = true);

	// Add an image to the container; this will not copy the image
	// Return the index for the image
	virtual int addImage(const cv::Mat& image);
    
    // Add an image to the container; this will not copy the image
    // Return the index for the image
    virtual int addImage(const b3di::TImage& image);

	// Get an image at index
	// read from a file if necessary
	virtual cv::Mat getImage(int index);

	// Get an image at index from the cache only (will not read from file)
	// Returns an empty image if the image is not in the cache
	virtual cv::Mat getCachedImage(int index);
    
    // Return a const reference to the cache in memory (TImage, compressed data)
    // Get an image at index from the cache only (will not read from file)
    // Returns an empty image if the image is not in the cache
    virtual b3di::TImage getCachedTImage(int index) const;

	// Get the last (most recently) cached image in the container
	// Image caching must be enabled first
	// @param [out] index The index of the image returned
	virtual cv::Mat getLastCachedImage(int& index);

	// Set an image at index in the container; this will not copy the image data
	virtual void setImage(int index, const cv::Mat& image);

	// Unset image at index. This removes a reference to the original image so it can be freed
	virtual void unsetImage(int index);

	// compression quality when saving image to file 
	// (0-9 for PNG, 0-100 for JPG). Use -1 for default (9: PNG, 90: JPG)
	virtual void setOutputImageQuality(int quality) { _quality = quality; }
	virtual int getOutputImageQuality() const { return _quality; }

	// 0: no rotation; 1: rotate 90 CW; -1: rotate 90 CCW; 2: rotate 180
	virtual void setInputImageRotation(int rotation) { _rotation = rotation;  }
	virtual int getInputImageRotation() const { return _rotation; }

	// Set the scale to be applied to the input image
	virtual void setInputImageScale(double scale) { _scale = scale; }
	virtual double getInputImageScale() const { return _scale; }

	// Enable caching of input images. cacheSize is the max number of images to store in the cache. 0 to cache all images if caching is true.
	// Set caching to false to disable caching (always read from file)
	// Set compress to true to cache compressed images (when caching is true)
	// Must use getImage for this to work
	virtual void setImageCaching(bool caching, int cacheSize = 0, bool compress=true);

	// Set image writing mode when an image is added to the container 
	// If enableWriting is true, an image is also written to the disk if output dir and file paths are set
	virtual void setImageWriting(bool enableWriting) { _writing = enableWriting; }

 //   // Set the Container Mode
 //   void setFileMode(bool isFileMode)
 //   {
 ////       _isFileMode = isFileMode;
 //   }

    // Should only call this function to get 'count'
	virtual int getCount();

	// Return the number of images
	virtual int getImagesCount() const;
	virtual ImageDataType getDataType() const;

	// static class function to write an image file
	// cant be called without a class oject
	// fileType can be "png" or "jpg"
	// quality: (0-9 for PNG, 0-100 for JPG). Use -1 for default (9: PNG, 95: JPG)
	static bool writeImage(const cv::Mat& image, const cv::String& imageFile, const cv::String& fileExt = IMAGE_PNG_SUFFIX, int quality = -1);

	static cv::Mat readImage(const cv::String& imageFile, ImageDataType imageType = IMAGE_RGB, int rotation = 0, double scale=1.0);

	// Release images in the container and clear the cache
	virtual void clear();

	/**
	* Shrink the image container size.
	*
	* @param [in] newSize new size. Ignore if it is larger than the current size.
	*/
	virtual void shrink(int newSize);

    // Get the current cache data size in bytes
    virtual size_t getCacheDataSize() const;

protected:

	// Add image to image cache
	// If index is >= 0, add image to the index position in the image cache
	virtual void addImageToCache(const cv::Mat& image, int index=-1);
	virtual void addImageToCache(const TImage& tImage, int index = -1);
	virtual cv::Mat removeImageFromCache(int index);
	static bool getCompressionParams(const cv::String& fileType, int quality, std::vector<int>& params);

	// read an image keep it compressed
	static TImage readImageCompressed(const cv::String& imageFile, ImageDataType imageType = IMAGE_RGB);
	static bool writeImageCompressed(const TImage& tImage, const cv::String& outputFile);
    static cv::Mat rotateImage(const cv::Mat& inputImage, int rotation);
    static cv::Mat scaleImage(const cv::Mat& inputImage, double scale);

private:

	ImageDataType _dataType;

	int _quality;
	int _rotation;
	bool _caching;
	double _scale;
	int _cacheSize;
	bool _writing;
	bool _compress;

	// indices to _images of cached images
	std::vector<int> _cacheIndices;

	// a list of cached images
	std::vector<TImage> _images;

	// Use this to indicate the current image index being written to the cache so that a getFrame will not try to access it
	// This assumes there is only one writer
	int _currentWriteIndex;

};

} // namespace b3di