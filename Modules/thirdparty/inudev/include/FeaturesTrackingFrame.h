/*
* File - CFeaturesTrackingFrame.h
*
* This file is part of the Inuitive SDK
*
* Copyright (C) 2014 All rights reserved to Inuitive  
*
*/

#ifndef __FEATURESTRACKINGFRAME_H__
#define __FEATURESTRACKINGFRAME_H__

#include "BaseStream.h"
#include <stdint.h>

namespace InuDev
{

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Represents generic buffer that is provided by some of InuDev streams 
    ///
    /// Responsibilities: 
    ///      1. Buffer attributes: data and size.
    ///      2. Memory management control.
    ///
    /// Comment: The interpretation of provided buffers should be done by the caller application.
    ///          The caller application should be familiar with the internal format of 
    ///          each provided frame.
    ////////////////////////////////////////////////////////////////////////
    class CFeaturesTrackingFrame : public CBaseFrame
    {

    public:
        
        /// \brief    Constructs empty (zero size) buffer
        INUSTREAMS_API CFeaturesTrackingFrame();

        /// \brief    Constructor.
        /// \param[in] bufferSize    generated buffer size.
        INUSTREAMS_API CFeaturesTrackingFrame(uint32_t bufferSize);

        /// \brief    Copy constructor - deep Copy.
        INUSTREAMS_API CFeaturesTrackingFrame(const CFeaturesTrackingFrame& input);

        /// \brief    Move constructor. 
        INUSTREAMS_API CFeaturesTrackingFrame(CFeaturesTrackingFrame&& input);

        /// \brief    Assignment operator - deep copy.
        INUSTREAMS_API CFeaturesTrackingFrame& operator=(const CFeaturesTrackingFrame& input);

        /// \brief    Move assignment operator.
        INUSTREAMS_API CFeaturesTrackingFrame& operator=(CFeaturesTrackingFrame&& input);

        /// \brief    Destructor.
        INUSTREAMS_API virtual ~CFeaturesTrackingFrame();

		/// \brief    Getter for buffer size
        INUSTREAMS_API unsigned int BufferSize() const  {  return mBufferSize;  }

		/// \brief    Getter for frame data
        INUSTREAMS_API const InuDev::byte* GetData() const {  return mData;  }

		/// \brief    Getter for frame data
        INUSTREAMS_API InuDev::byte*  GetData() {  return mData;  }

    protected:

        ///  \brief     Data buffer
        InuDev::byte* mData;

        ///  \brief     Buffer size
        unsigned int mBufferSize;

        ///  \brief    If data was extracted (by using ExtractDataBuffer) then this flag becomes false and data will not be released.
        bool mReleaseData;
    };

}

#endif // __FEATURESTRACKINGFRAME_H__
