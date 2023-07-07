/*
* File - ImageFrame.h
*
* This file is part of the Inuitive SDK
*
* Copyright (C) 2014 All rights reserved to Inuitive  
*
*/

#ifndef __IMAGEFRAME_H__
#define __IMAGEFRAME_H__

#include "StreamsExport.h"

#include "BaseStream.h"
#include "InuSensor.h"

namespace InuDev
{

    struct CImageFrameParams
    {
        unsigned short  channelId;
        unsigned short  frameStartX;
        unsigned short  frameStartY;
        unsigned short  frameWidth;
        unsigned short  frameHeight;
    };

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Represents  an image that is  provided  by InuDev streams 
    ///
    /// Responsibilities: 
    ///      1. Image attributes: format, scale, width, height and number of bytes per pixel.
    ///      2. Knows how to manage the image buffer.
    ///
    ////////////////////////////////////////////////////////////////////////
    class CImageFrame : public CBaseFrame
    {

    public:

        ///  \brief     Supported formats
        enum EFormat 
        { 
            eEmpty = 0,					///<  Empty buffer      
            eDepth,						///<  2 Bytes per pixel: Z-Buffer (16 bits per pixel)  
            eBGR,						///<  3 Bytes per pixel: B, G and R 
            eBGRA,						///<  4 Bytes per pixel: B, G, R and Alpha (which is always 0xff - 100%)
            eDisparity,					///<  2 Bytes per pixel: 4 MSB of confidence and 12 LSB of Disparities 
            eRGB565,					///<  2 Bytes per pixel: Standard RGB565 format  
            eDisparityB0,				///<  2 Bytes per pixel: 12 MSB of Disparities and 4 LSB of confidence  
            eRGBA,						///<  4 Bytes per pixel: R, G, B and Alpha (which is always 0xff - 100%)
            eDepthWithConfidence,		///<  2 Bytes per pixel: 14 MSB of depth and 2 LSB of confidence (the higher the better)
            ePointCloud,				///<  3 floats per pixel: X, Y, Z of point cloud 
            eYUV422,					///<  2 bytes per pixel: Compressed Y, U, V 
            eABGR,						///<  4 Bytes per pixel: Alpha, B, G, R (Alpha which is always 0xff - 100%)
            eARGB,						///<  4 Bytes per pixel: Alpha, R, G, B (Alpha which is always 0xff - 100%)
            eRGB,						///<  3 Bytes per pixel: R, G and B 
            ePointCloudWithConfidence,  ///<  4 floats per pixel: X, Y, Z of point cloud + 1 float of confidence 

			eYUV =  100,				///<  2 Bytes per pixel: packed Y, U, V 
			eY,							///<  2 Bytes per pixel: 10 bits Y (the 11th bit indicates overflow)
			eUnpackedYUV,				///<  3 Bytes per pixel: Y, U, V
			eYUVFloat,					///<  3 floats per pixel: Y, U, V
			eBayerGRBG,					///<  2 Bytes per pixel: Bayer GRBG
			eBayerRGGB,					///<  2 Bytes per pixel: Bayer RGGB
			eBayerBGGR,					///<  2 Bytes per pixel: Bayer BGGR
			eBayerGBRG,					///<  2 Bytes per pixel: Bayer GBRG
			eDepthA0,					///<  2 Bytes per pixel: Depth from HW: 5 bits of confidence (MSB)
			eDepthB0,					///<  2 Bytes per pixel: Depth from HW: 14 MSB of depth and 2 LSB of confidence (the higher the better)
			eY8,						///<  1 Bytes per pixel: 8 bit Y 
        };

        /// \brief    Scale factor (relevant only to eDepth format)
        enum EScale
        { 
            eMM = 0,    ///< Default scale (mm)
            eCM 
        };

        /// \brief    Constructs empty (zero size) frame
        INUSTREAMS_API CImageFrame();

        /// \brief    Constructor.
        /// \param[in] iWidth         Image width.       (full buffer width)
        /// \param[in] iHeight        Image height.
        /// \param[in] iImageWidth    Image width.       (real size of image inside the buffer)
        /// \param[in] iImageHeight   Image height.
        /// \param[in] iStartX        X offset inside the buffer
        /// \param[in] iStartY        Y offset inside the buffer
        /// \param[in] iFormat        Image format (which defines  the number of bytes  per pixel).
        /// \param[in] iMinDisparity        
        /// \param[in] iMaxDisparity      
        INUSTREAMS_API CImageFrame(unsigned int iWidth,
            unsigned int iHeight,
            unsigned int iImageWidth,  
            unsigned int iImageHeight,
            unsigned short iStartX,
            unsigned short iStartY,
            unsigned int iFormat,
            unsigned int iMinDisparity = 0,
            unsigned int iMaxDisparity = 0);
        

        /// \brief    Constructor.
        /// \param[in] iData          Data buffer.
        /// \param[in] iWidth         Image width.      (full buffer width)
        /// \param[in] iHeight        Image height.
        /// \param[in] iImageWidth    Image width.      (real size of image inside the buffer)
        /// \param[in] iImageHeight   Image height.
        /// \param[in] iStartX        X offset inside the buffer
        /// \param[in] iStartY        Y offset inside the buffer
        /// \param[in] iFormat    Image format (which defines  the number of bytes  per pixel).
        /// \param[in] iOwnData    If this flag is true then this CImageFrame object is responsible for freeing the buffer
        INUSTREAMS_API CImageFrame(InuDev::byte*  iData,
            unsigned int iWidth,
            unsigned int iHeight,
            unsigned int iImageWidth, 
            unsigned int iImageHeight,
            unsigned short iStartX,
            unsigned short iStartY,
            unsigned int iFormat,
            bool iOwnData = true); 

        /// \brief    Copy constructor - deep Copy.
        INUSTREAMS_API CImageFrame(const CImageFrame& input);

        /// \brief    Move constructor. 
        INUSTREAMS_API CImageFrame(CImageFrame&& input);

        /// \brief    Assignment operator - deep copy.
        INUSTREAMS_API CImageFrame& operator=(const CImageFrame& input);

        /// \brief    Move assignment operator.
        INUSTREAMS_API CImageFrame& operator=(CImageFrame&& input);

        /// \brief    Destructor.
        INUSTREAMS_API virtual ~CImageFrame();

		/// \brief    Image Size - number of pixels
		unsigned int ImageSize() const { return mWidth * mHeight; }

		/// \brief    Image buffer size in bytes
		unsigned int BufferSize() const { return ImageSize() * BytesPerPixel(); }

		/// \brief    Image width  
		unsigned int Width() const { return mWidth; }

		/// \brief    Image height 
		unsigned int Height() const { return mHeight; }

		/// \brief    Width of image that is cropped by depth engine, is relevant when the image padding is requested. CroppedImageWidth <=  Width
		unsigned int CroppedImageWidth() const { return mCroppedImageWidth; }

		/// \brief    Height of image  that is cropped by depth engine, is relevant when the image padding is requested. CroppedImageHeight <=  Height
		unsigned int CroppedImageHeight() const { return mCroppedImageHeight; }

		/// \brief    Top left width offset of cropped image, is relevant when the image padding is requested. CroppedImageTopLeftW >= 0
		unsigned short CroppedImageTopLeftW() const { return mCroppedImageTopLeftW; }

		/// \brief    Top left height offset of cropped image, is relevant when the image padding is requested. CroppedImageTopLeftH >= 0
		unsigned short CroppedImageTopLeftH() const { return mCroppedImageTopLeftH; }

		/// \brief    Image format
        unsigned int Format() const    { return mFormat; }

        /// \brief    Number of bytes that are used to represent each pixel.
        INUSTREAMS_API unsigned int BytesPerPixel() const;

        EScale Scale() const { return mScale; }
        EScale& Scale() { return mScale; }

		/// \brief    Non mutable pointer to image data
        const InuDev::byte* GetData() const  { return mData; }

		/// \brief    Pointer to image data
		InuDev::byte*  GetData() { return mData; }

		/// \brief    Exposure parameters while this image was grabbed  
		CExposureParams GetExposureParams() const { return mExposureParams; }

        /// \brief    ExtractDataBuffer() - Extracts the data buffer from the ImageFrame 
        /// 
        /// Returns the ImageFrame's data buffer, and removes the ownership.
        /// The ImageFrame won't hold the buffer anymore, and is no longer responsible for freeing the buffer.
        /// \return InuDev::byte*     data buffer pointer
        INUSTREAMS_API InuDev::byte*   ExtractDataBuffer();

        /// \brief    Number of invalid pixels received by depth calculation 
        uint32_t GetNumOfInvalidDepthPixels() const { return  mNumOfInvalidDepthPixels; }

        unsigned int                    MinDisparity() const { return mMinDisparity; }
        void                            MinDisparity(unsigned int val) { mMinDisparity = val; }

        unsigned int                    MaxDisparity() const { return mMaxDisparity; }
        void                            MaxDisparity(unsigned int val) { mMaxDisparity = val; }


         
    protected:

		/// \brief     Data buffer
		InuDev::byte* mData;
		
		/// \brief     Image width (full buffer width) 
        unsigned int mWidth;

        /// \brief     Image height (full buffer height)
        unsigned int mHeight;

		/// \brief    Width of image that is cropped by depth engine, is relevant when the image padding is requested. CroppedImageWidth <= Width
		unsigned int mCroppedImageWidth;

		/// \brief    Height of image  that is cropped by depth engine, is relevant when the image padding is requested. CroppedImageHeight <= Height
		unsigned int mCroppedImageHeight;

		/// \brief    Top left width offset of cropped image, is relevant when the image padding is requested. CroppedImageTopLeftW >= 0
		unsigned short mCroppedImageTopLeftW;

		/// \brief    Top left height offset of cropped image, is relevant when the image padding is requested. CroppedImageTopLeftH >= 0
		unsigned short mCroppedImageTopLeftH;
		
        /// \brief     Image Format
        unsigned int mFormat;

        /// \brief     Scale (relevant only to eDepth format)
        EScale mScale;

        /// \brief    If data was extracted (by using ExtractDataBuffer) then this flag becomes false and data will not be released.
        bool mReleaseData;

        /// \brief    Holds the ET, digital and analog gain L/F
        CExposureParams mExposureParams;

		/// \brief    Number of invalid pixels received by depth calculation
        uint32_t  mNumOfInvalidDepthPixels;

        unsigned int mMinDisparity;

        unsigned int mMaxDisparity;
    };
}

#endif // __IMAGEFRAME_H__
