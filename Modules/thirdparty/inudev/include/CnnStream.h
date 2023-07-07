#ifndef __CNNSTREAMH__
#define __CNNSTREAMH__

#include "StreamsExport.h"
#include "BaseStream.h"
#include "GeneralFrame.h"

#include <functional>
#include <string>

namespace InuDev 
{
    /// \brief    Load CNN parameters
    struct CCnnLoadParams
    {
        /// \brief	Network unique ID
        std::string NetworkID;
        
        /// \brief	Network is loaded from this file.
        std::string NetworkFilename;
        
        CCnnLoadParams() : NetworkID(), NetworkFilename() {}
    };

    /// \brief    Start CNN parameters
    struct CCnnStartParams
    {
        /// \brief	Network unique ID
        std::string NetworkID;

        /// \brief	Injection mode: true if input images for CNN are injected and not live images
        bool InjectionMode;	

        /// \brief	Pixel size in case of injection
        uint32_t InjectedPixelSize; 

        /// \brief	Image width in case of injection
        uint32_t InjectedImageWidth;

        /// \brief	Image height in case of injection
        uint32_t InjectedImageHeight; 
        
        /// \brief	Maximum images that can be injected at once
        uint32_t MaxInjectedImages;

        CCnnStartParams() : NetworkID(), InjectionMode(false) {}
    };

    ////////////////////////////////////////////////////////////////////////
    /// \brief    General CNN Stream
    ///
    /// Role: Load/run and analyze a CNN on device.  
    ///
    /// Responsibilities: 
    ///      1. control the service (Init, Terminate, Start and Stop).
    ///      2. control CNN on the device (Load, Start, Stop)
    ///      3. provide a continuous stream of result frames
    ///
    ////////////////////////////////////////////////////////////////////////
    class CCnnStream  : public CBaseStream 
    {
    public:

        typedef std::function<void(std::shared_ptr<CCnnStream>, std::shared_ptr<const CGeneralFrame>,  CInuError)> CallbackFunction;

        virtual ~CCnnStream() {}

        /// \brief    Retrieves new CNN image frame (pull) 
        /// 
        /// This method returns when a new frame is ready (blocking) or if an input timeout has elapsed. 
        /// It shall be called only after a Start() was is invoked but before any invocation of a Stop() is invoked.
        /// \param[out] oCNNFrame    The returned CNN frame (char buffer) that can be analyzed by the caller application.
        /// \param[in] iTimeout    Function is returned if timeout has elapsed even if no new frame is ready.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError GetFrame(std::shared_ptr<const CGeneralFrame>& oCNNFrame, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        /// \brief    Registration/Unregistration for receiving stream of CNN frames (push) 
        /// 
        /// The provided callback function is called when a new frame is ready (non-blocking). 
        /// It shall be called only after a Start() was is invoked but before any invocation of a Stop() is invoked.
        /// \param[in] iCallback    Callback function which is invoked when a new frame is ready. 
        ///                         Send nullptr to unregister for receiving frames.  
        /// \param[in] iTimeout    Callback is invoked if a timeout has elapsed even if no new frame is ready.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError Register(CallbackFunction iCallback, unsigned int iTimeout = FPS_BASED_TIMEOUT) = 0;

        /// \brief    Load network from input file name.
        /// \param[in] iLoadParams    Loaded network parameters.
        /// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError LoadNetwork(const CCnnLoadParams& iLoadParams) = 0;

        /// \brief    Start CNN identified by its unique ID.
        /// \param[in] iStartParams    Started network parameters.
        /// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError StartNetwork(const CCnnStartParams& iStartParams) = 0;

        /// \brief    Stop a running CNN.
        /// \param[in] iNetworkID    Network unique ID.
        /// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError StopNetwork(const std::string& iNetworkID) = 0;
        
        /// \brief    Write buffer to CNN service.
        /// \param[in] iNetworkID    Network unique ID.
        /// \param[in] iBuffer    Transfered data.
        /// \param[in] iBufferLen    Transferred buffer length.
        /// \param[in] iImages    Number of images in buffer.
        virtual CInuError WriteData(const std::string& iNetworkID, const unsigned char* iBuffer, unsigned int iBufferLen, unsigned int iImages) = 0;


        /// \brief    SetNetworkAttributes.
        /// \param[in] iNetworkID    Network unique ID.
        /// \param[in] iBuffer    Attributes Transfered data.
        /// \param[in] iBufferLen    Attributes Transferred buffer length.
        virtual CInuError SetNetworkAttributes(const std::string& iNetworkID, const unsigned char* iBuffer, unsigned int iBufferLen) = 0;


        /// \brief   Release all previously loaded CNN networks.
        /// \return InuDev::CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError ReleaseNetworks(void) = 0;
    };
}

#endif // __CNNSTREAM_H__
