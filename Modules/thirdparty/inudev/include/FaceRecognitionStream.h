#ifndef __FACERECOGNITIONSTREAM_H__
#define __FACERECOGNITIONSTREAM_H__



#include "StreamsExport.h"

#include "BaseStream.h"
#include <functional>

namespace InuDev 
{

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Face frame provided by  CFaceRecognitionStream
    ///
    /// Role: Face position (User ID is defined in CBaseFrame)
    ////////////////////////////////////////////////////////////////////////  
    ////////////////////////////////////////////////////////////////////////
    /// \brief    Frame of all detected objects by the running CNN
    ///
    /// Role: Collection of all objects that are detected by CNN
    ////////////////////////////////////////////////////////////////////////  
    struct CFaceRecognitionFrame : public CBaseFrame
    {
        /// \brief	Face position  
        enum EFacePose
        {
            eNone,		///< Unrecognized position	 
            eLeft,		///< Looking left (Horizontal)	 
            eRight,		///< Looking right (Horizontal)
            eTop,		///< Looking top (Vertical)	
            eBottom,	///< Looking bottom (Vertical)	
            eCenter		///< Looking to the center (Horizontal/Vertical)	
        };

        struct CRecognizedFace
        {
            /// \brief	Classification identifier of this object
            std::string FaceID;

            /// \brief	Confidence score of this object
            float Confidence;

            /// \brief	Top Left coordinates of detected object.
            CPoint2D ClosedRectTopLeft;

            /// \brief	Size of recognized face.
            CPoint2D ClosedRectSize;

            /// \brief	Horizontal face position (Left/Right/Center)
            EFacePose PoseH;

            /// \brief	Vertical face position (Top/Bottom/Center)
            EFacePose PoseV;

            /// \brief	Number of landmark points
            static const int LANDMARKS_POINTS = 5;

            /// \brief	landmark points (eyes, nose, mouth)
            CPoint<2,uint32_t> Landmarks[LANDMARKS_POINTS];
        };

        /// \brief		Collection of all detected objects
        std::vector<CRecognizedFace> RecognizedFaces;
    };
    

    ////////////////////////////////////////////////////////////////////////
    /// \brief   Interface for Face Recognition service.
    /// 
    /// Role: Controls Face Recognition service and provides Face frames.  
    ///
    /// Responsibilities: 
    ///      1. Knows how to control the service (Init, Terminate, Start and Stop).
    ///      2. Knows how to acquire one face frame (pull)
    ///      3. Knows how to provide a continuous stream of face frames (push)
    ///      4. Enroll new face and remove old face from Faces DB
    ///
    ////////////////////////////////////////////////////////////////////////
    class CFaceRecognitionStream  : public CBaseStream 
    {
    public:

        typedef std::function<void(std::shared_ptr<CFaceRecognitionStream>, std::shared_ptr<const CFaceRecognitionFrame>,  CInuError)> CallbackFunction;

        virtual ~CFaceRecognitionStream() {}

        virtual CInuError         GetFrame(std::shared_ptr<const CFaceRecognitionFrame>& oHeadFrame, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        virtual CInuError         Register(CallbackFunction iCallback, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        /// \brief    Add new face to faces Database. This process might take time since there is a need to train the CNN.
        /// \param[in] iFaceID    Face ID to add.
        /// \param[in] iImagesDir    Full path to folder that contains images of the new face to be added.
        virtual CInuError         Enroll(const std::string& iFaceID, const std::string& iImagesDir) = 0;

        /// \brief    Remove Face from faces database.
        /// \param[in] iFaceID    Face ID to remove.
        virtual CInuError         Remove(const std::string& iFaceID) = 0;

    };
}

#endif // __FACERECOGNITIONSTREAM_H__
