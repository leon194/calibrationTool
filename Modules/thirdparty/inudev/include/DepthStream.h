/*
* File - DepthStream.h
*
* This file is part of the Inuitive SDK
*
* Copyright (C) 2014 All rights reserved to Inuitive  
*
*/


#ifndef __DEPTHSTREAM_H__
#define __DEPTHSTREAM_H__

#include "ImageFrame.h"
#include "BaseStream.h"
#include <functional>

namespace InuDev 
{

    ///////////////////////////////////////////////////////////////////////
    /// \brief    All different Post Preprocessing algorithms
	///			  That's a bitwise mask so multiple modes are allowed
    ///////////////////////////////////////////////////////////////////////
    enum EPostProcessingMode
    {
        eDisabled = 0,          ///<  No post processing algorithm is executed (best runtime)
        eCropOnlyMode = 1,      ///<  Crops edges and disparities (very fast)     
        eFastMode = 2 ,         ///<  Removes singular points (fast)
        eAccurate = 4,          ///<  Removes singular points + deblocking filter (moderated runtime performance)
		eBlob = 8,				///<  NU4000 Blob 
		ePlaneFit = 16,			///<  NU4000 Plane Fit 
		eConfidence = 32,		///<  NU4000 Confidence 
		eInformationMap = 64,   ///<  NU4000 Information Map
	};

    ////////////////////////////////////////////////////////////////////////
    /// \brief   Interface for Depth service.
    /// 
    /// Role: Controls depth images streaming service and provides depth image frames. 
    ///
    /// Responsibilities: 
    ///      1. Derives CBaseStream interface
    ///      2. Knows how to acquire one depth image frame (pull)
    ///      3. Knows how to provide a continuous stream of depth image frames (push)
    /// 
    ////////////////////////////////////////////////////////////////////////
    class CDepthStream : public CBaseStream
    {
    public:

        enum EOutputType
        {
			eDepth = 0,		///<  Provides CImageFrames of CImageFrame::EFormat::eDepth or CImageFrame::EFormat::eDepthWithConfidence
			eDisparity = 1,		///<  Provides CImageFrames of CImageFrame::EFormat::eDisparity or CImageFrame::EFormat::eDisparityB0
			ePointCloud = 2,	///<  Provides CImageFrames of CImageFrame::EFormat::ePointCloud or CImageFrame::EFormat::ePointCloudWithConfidence
			eRGB = 3,	///<  Provides colored Z-map, blue for high Z values and red for low Z values  (CImageFrame::EFormat::eRGB)  
			eDepthRegistration = 4,		///<  Projection of depth image on webcam, format is similar to eDepth 
			eRaw = 5,	///<  Provides CImageFrames as are streamed by Inuitive's chip, no additional processing on Host (generally its same as eDisparity)
        };

        /// \brief    Prototype of callback function which is used by the Register method.
        ///
        /// This function is invoked any time a frame is ready, or if an error occurs. The parameters of this function are:
        /// Caller stream object, received depth frame and result code.
        typedef std::function<void(std::shared_ptr<CDepthStream> , std::shared_ptr<const CImageFrame>,  CInuError)> CallbackFunction;

        virtual ~CDepthStream() {}

        /// \brief    Derived interface, Init without initial parameters
        using CBaseStream::Init;

        /// \brief    Service initialization.
        /// 
        /// Shall be invoked once before starting frames acquisition.
        /// \param[in] iWithConfidence    Acquire depth frames with confidence.
        /// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError         Init(bool iWithConfidence) = 0;

		/// \brief    Service initialization.
		/// 
		/// Shall be invoked once before starting frames acquisition.
		/// \param[in] iOutputType    Output format of acquired frames.
		/// \param[in] iWithConfidence    Acquire depth frames with confidence.
		/// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
		virtual CInuError         Init(EOutputType iOutputType, bool iWithConfidence = false) = 0;
		
		/// \brief    Service initialization.
		/// 
		/// Shall be invoked once before starting frames acquisition.
		/// \param[in] iOutputType    Output format of acquired frames.
		/// \param[in] iPostProcessingMode    Bit mask of all Post Processing algorithms that should be invoked.
		/// \param[in] iWithConfidence    Acquire depth frames with confidence.
		/// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
		virtual CInuError         Init(EOutputType iOutputType, unsigned int iPostProcessingMode, bool iWithConfidence = false) = 0;

        /// \brief    Retrieves new depth image frame (pull) 
        /// 
        /// This method returns when a new frame is ready (blocking) or if an input timeout has elapsed. 
        /// It shall be called only after a Start() was is invoked but before any invocation of a Stop() is invoked.
        /// \param[out] oDepthFrame    The returned depth frame (Z-buffer).
        /// \param[in] iTimeout    Function is returned if timeout has elapsed even if no new frame is ready.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError  GetFrame(std::shared_ptr<const CImageFrame>& oDepthFrame, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        /// \brief    Registration/Unregistration for receiving stream of depth image frames (push) 
        /// 
        /// The provided callback function is called when a new frame is ready (non-blocking). 
        /// It shall be called only after a Start() was is invoked but before any invocation of a Stop() is invoked.
        /// \param[in] iCallback    Callback function which is invoked when a new frame is ready. 
        ///                         Send nullptr to unregister for receiving frames.  
        /// \param[in] iTimeout    Callback is invoked if a timeout has elapsed even if no new frame is ready.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError Register(CallbackFunction iCallback, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        /// \brief SetPointCloudRatio()
        /// 
        /// Detailed description:        Send the user point cloud FPS. by default is set to be same as the depth frame FPS 
        /// \param[in] iFPS              The new fps
        /// \return InuDev::CInuError    Return error if user sets the fps to be 0 or greater than the depth fps, 
        ///                              otherwise, it is ok.
        virtual CInuError         SetPointCloudFPS(unsigned int iFPS ) = 0;

    };
}

#endif
