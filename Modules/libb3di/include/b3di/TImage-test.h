#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <future>

namespace b3di {


// A simple header only class to encapsulate compressed and uncompressed cv::Mat
class TImage
{
public:

	TImage(const cv::Mat& image, bool compress=false, const cv::String& imageExt=".png", 
		const std::vector<int>& encodingParams= std::vector<int>(), bool async=true)
	{
		if (!compress) {
			_image = image;
            _compressed = compress;
        }
        else {

            if (async) {
                std::async(std::launch::async, compressImage, imageExt, image, encodingParams, _compData);
                //std::async(std::launch::async, [](bool& compressed, const cv::String& imgExt, const cv::Mat& img,
                //    std::vector<uchar>& data, const std::vector<int>& params)
                //{ if (cv::imencode(imgExt, img, data, params)) { compressed = true; }  },
                //    _compressed, imageExt, image, _compData, encodingParams);
            }
            else {
                cv::imencode(imageExt, image, _compData, encodingParams);
                //cv::Mat img = cv::imdecode(_compData, CV_LOAD_IMAGE_UNCHANGED);
                //std::cout << img.size() << std::endl;
                _compressed = compress;
            }
		}
    };
	TImage(const std::vector<uchar>& compData)
	{
		_compressed = true;
		_compData = compData;
	}
	TImage() {
		_compressed = false;
	}

	virtual ~TImage() {}

	cv::Mat getImage(int imageFlag= cv::IMREAD_UNCHANGED) {
		if (!_compressed) {
			return _image;
		}
		else {
			cv::Mat img = cv::imdecode(_compData, imageFlag);
			//std::cout << img.size() << std::endl;
			return img;
		}
	}

	std::vector<uchar> getCompressedData() const { return _compData; }

	bool compressed() const {
		return _compressed;
	}

	bool empty() const {
		return _compressed?_compData.empty():_image.empty();
	}

	void release() {
		_image.release();
		_compData.clear();
	}

    // return the image data size in bytes
    size_t getDataSize() const {
        if (_compressed) {
            return _compData.size();
        }
        else {
            //https://stackoverflow.com/questions/26441072/finding-the-size-in-bytes-of-cvmat
            return _image.step[0] * _image.rows;
        }
    }

    static void compressImage(const cv::String& imgExt, const cv::Mat& img, const std::vector<int>& params, std::vector<uchar>& data) {
        if (cv::imencode(imgExt, img, data, params)) { 
        }
    }

private:

	bool _compressed;					// flag to indicate if the image is compressed or not
	cv::Mat _image;						// uncompressed image
	std::vector<uchar> _compData;		// compressed image data

};

} // namespace b3di

