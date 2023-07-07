#ifndef __OBJECTSDETECTIONSTREAMH__
#define __OBJECTSDETECTIONSTREAMH__

#include "StreamsExport.h"
#include "BaseStream.h"

#include <functional>
#include <string>
#include <vector>

namespace InuDev 
{

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Frame of all detected objects by the running CNN
    ///
    /// Role: Collection of all objects that are detected by CNN
    ////////////////////////////////////////////////////////////////////////  
    struct CObjectsDetectionFrame : public CBaseFrame
    {
        struct CDetectedObject
        {
            /// \brief		Classification identifier of this object
            std::string ClassID;

            /// \brief		Confidence score of this object
            float Confidence;

            /// \brief		Top Left coordinates of detected object.
            CPoint2D ClosedRectTopLeft;

            /// \brief		Size of recognized face.
            CPoint2D ClosedRectSize;
        };

        /// \brief		Collection of all detected objects
        std::vector<CDetectedObject> DetectedObjects;
    };

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Objects Detection implemented by CNN
    ///
    /// Role: Controls Objects Detection service and provides of all detected objects.  
    ///
    /// Responsibilities: 
    ///      1. Knows how to control the service (Init, Terminate, Start and Stop).
    ///      2. Knows how to acquire one frame of all detected objects (pull)
    ///      3. Knows how to provide a continuous stream of frames of all detected objects (push)
    ///
    ////////////////////////////////////////////////////////////////////////
    class CObjectsDetectionStream  : public CBaseStream 
    {
    public:

        typedef std::function<void(std::shared_ptr<CObjectsDetectionStream>, std::shared_ptr<const CObjectsDetectionFrame>,  CInuError)> CallbackFunction;

        virtual ~CObjectsDetectionStream() {}

        virtual CInuError         GetFrame(std::shared_ptr<const CObjectsDetectionFrame>& oHeadFrame, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        virtual CInuError         Register(CallbackFunction iCallback, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

    };
}

#endif // __ObjectsDetectionSTREAM_H__
