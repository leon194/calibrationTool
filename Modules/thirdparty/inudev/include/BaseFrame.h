/*
* File - BaseFrames.h
*
* This file is part of the Inuitive SDK 
*
* Copyright (C) 2014 All rights reserved to Inuitive  
*
*/


#ifndef __BASEFRAME_H__
#define __BASEFRAME_H__

#include "StreamsExport.h"

#include "StreamsExport.h"
#include "InuError.h"
#include "InuDefs.h"

#include <time.h> 
#include <string>
#include <vector>
#include <limits>
#include <stdint.h>

#ifdef max
#undef max
#endif

namespace InuDev
{

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Common behavior of all InuDev NUI frames. 
    ///
    /// Role: Base class of all InuStreams' frames.
    ///
    /// Responsibilities: 
    ///      1. Common members. 
    ///      2. Construction is enabled only from derived classes.
    ///
    ////////////////////////////////////////////////////////////////////////
    class  CBaseFrame 
    {
    public:
        // Upper and lower limits of Score 
        static const unsigned int SCORE_MIN=0;
        static const unsigned int SCORE_MAX=100;

        /// \brief    Frame acquisition time in nanoseconds relates to host's system clock.
        ///           It should be used for frames synchronization. 
        uint64_t Timestamp;

        /// \brief    Frame acquisition unique index, should be used for synchronization. 
        uint32_t FrameIndex;

        /// \brief    Indicates if this frame is valid data or not. 
        bool Valid;

        /// \brief    Confidence of current frame; range is from SCORE_MIN up to SCORE_MAX.
        unsigned int Score; 

        /// \brief    Unique ID of the user who is the source of this NUI frame. String is empty when user is not detected. 
        std::string UserID;

        /// \brief    Time stamp which is set when frame is received from USB (used for statistics and analysis).
        uint64_t ServiceTimestamp;

        /// \brief    Time stamp which is set when frame is received from InuService (used for statistics and analysis).
        uint64_t StreamTimestamp;

        /// \brief    true if this frame was recorded by InuService
        bool WasRecorded;

        /// \brief    Calibration temperature which was applied while this frame was acquired. std::numeric_limits<int32_t>::max() if it is not valid. 
        int32_t CalibrationTemperature;

        /// \brief    Active projector recieved by FW for this frame: 0 - default(Pattern), 1 - Flood projector.
        int32_t ActiveProjector;

        virtual ~CBaseFrame() {}

    protected:

        /// \brief    Constructor
        /// \param[in] iTime    Frame acquisition time.
        /// \param[in] iFrameIndex    Frame acquisition unique index.
        /// \param[in] iScore    Confidence.
        /// \param[in] iValid    Validity bit.
        /// \param[in] iUserID    Unique User ID.
        CBaseFrame(uint64_t iTime = 0,
            unsigned int iFrameIndex = 0,
            unsigned int iScore = SCORE_MAX,
            bool iValid = false,
            std::string iUserID = std::string()) :
            Timestamp(iTime),
            FrameIndex(iFrameIndex),
            Valid(iValid),
            Score(SCORE_MAX),
            UserID(iUserID),
            ServiceTimestamp(0),
            StreamTimestamp(0),
            CalibrationTemperature(std::numeric_limits<int32_t>::max()),
            WasRecorded(false)
        {
        }
    };
}

#endif // __BASEFRAME_H__
