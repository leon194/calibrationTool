/*
 * File - Provider.h
 *
 * This file is part of the Inuitive SDK
 *
 * Copyright (C) 2014 All rights reserved to Inuitive  
 *
 */

#ifndef __INUSENSOR_H__
#define __INUSENSOR_H__

#include "StreamsExport.h"
#include "InuError.h"
#include "BaseStream.h"
#include "InuDefs.h"
#include "OpticalData.h"

#include <map>
#include <functional>
#include <stdint.h>

namespace InuDev 
{
    // Forward decelerations
    class CDepthStream;
    class CVideoStream;
    class CWebCamStream;
    class CImuStream;
    class CGeneralPurposeStream;
    class CAudioStream;
    class CFeaturesTrackingStream;
    class CSlamStream;
    class CObjectsDetectionStream;
    class CCnnStream;
    class CFaceRecognitionStream;

    ///////////////////////////////////////////////////////////////////////
    /// \brief    Information about SW and HW versions
    ///////////////////////////////////////////////////////////////////////
    struct CEntityVersion
    {
        /// \brief    Each version entity has a unique identification number
        enum EEntitiesID
        {
            eSerialNumber,			///<  Device Serial Number 
            eModelNumber,			///<  Device Model Number
            ePartNumber,			///<  Device Part Number
            eRevisionNumber,		///<  Device Revision Number
            eMasterID,				///<  Master sensor ID
            eSlaveID,				///<  Slave sensor ID
            eFWVersion,				///<  Firmware
            eHWVersion,				///<  Hardware version, returned values are defined by EHWVersion enumerator 
            eStreams,				///<  InuStreams.dll 
            eServices,				///<  InuService.exe
            eHWRevision,			///<  Hardware revision (0x3000 for NU3000 or 0x4000 for NU4000)
            eCalibrationRevision,   ///<  Calibration data revision
            eBootfixTimestamp,		///<  Boot fix time stamp
            eBootfixVersion,		///<  Boot fix Version
            eBootID,				///<  ID of boot that is in use
            eDualSensorsMaster,		///<  Indication if this is the Master of Dual Sensors configuration
            eISPVersion,            ///<  ISP version
            eCalibrationTemperature,///<  Temperature of current calibration that is in use (or std::numeric_limits<int>::max() if temperature isn't defined) 
            eTriggerMode            ///<  True if system is working in trigger mode where the image acquisition is done according to external trigger
        };

        /// \brief    Version Entity ID
        EEntitiesID ID;

        /// \brief    Name of this entity
        std::string Name;

        /// \brief    Version number 
        unsigned int VersionNum;

        /// \brief    Version description 
        std::string VersionName;
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    HW revisions (provided by CEntityVersion::eHWRevision) 
    ///////////////////////////////////////////////////////////////////////
    
    enum EHWVersion
    {
        eA0  = 0,          ///<  First chip generation 
        eB0 = 1,           ///<  Second chip generation 
        eUnknown
    }; 
    
    ///////////////////////////////////////////////////////////////////////
    /// \brief    All types of cameras that can be assembled in InuSensor
    ///////////////////////////////////////////////////////////////////////
    enum ECameraName
    {
        eVideo = 0,				///<  Stereo camera which is used for Depth calculation 
        eWebCam = 2,			///<  RGB or Fisheye camera
        eAllCameras = 100       ///<  limited to 255
    };


    ///////////////////////////////////////////////////////////////////////
    /// \brief    All possible states of connection with Sensor 
    ///////////////////////////////////////////////////////////////////////
    enum EConnectionState
    {
        eUnknownConnectionState = 0,	///<  Initial state, connection has not been established
        eConnected = 1,					///<  Sensor is connected 
        eDisconnected = 5,				///<  No Sensor is connected
        eServiceDisconnected = 4,		///<  Can't communicate with InuService
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    Configuration of current connected InuSesnor  (read only).
    ///////////////////////////////////////////////////////////////////////
    struct CSensorConfiguration
    {
        /// \brief    Video (Stereo camera) Image size that is provided by the this sensor [Width,Height] according to channelID. 
        std::map<unsigned int, CPoint<2, unsigned int>> VideoSize;

        /// \brief    Depth Image size that is provided by the this sensor [Width,Height] according to channelID.  
        std::map<unsigned int, CPoint<2, unsigned int>> DepthSize;

        /// \brief    Webcam Image size that is provided by this sensor [Width,Height]. 
        std::map<unsigned int, CPoint<2, unsigned int>> WebCamSize;
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    All resolutions supported by Inuitive Sensor
    ///////////////////////////////////////////////////////////////////////
    enum ESensorResolution
    {
        eDefaultResolution = 0,		///< Sensor default resolutions
        eBinning = 1,				///< Sensor's binning mode (reduced resolution provided by sensor)
        eVerticalBinning = 2,		///< Vertical binning resolution 
        eFull = 3,					///< Full sensor resolution  
        eAlternate = 4,				///< Alternating resolutions
    }; 


    ///////////////////////////////////////////////////////////////////////
    /// \brief    Sensor parameters 
    ///////////////////////////////////////////////////////////////////////
    struct CSensorParams
    { 
        static const int USE_DEFAULT_FPS = -1;

        /// \brief    Sensor Resolution
        ESensorResolution SensorRes;   

        /// \brief    Frame rate (number of frames per second)
        float FPS;         

        /// \brief    Default constructor which defines the default FPS and resolution
        CSensorParams() : SensorRes(eDefaultResolution), FPS(float(USE_DEFAULT_FPS)) {}
    };


   
    ///////////////////////////////////////////////////////////////////////
    /// \brief    Exposure parameters  
    ///////////////////////////////////////////////////////////////////////
    struct CExposureParams 
    {
        /// \brief    Exposure Time of the Right sensor
        uint32_t    ExposureTimeRight;

        /// \brief    Exposure Time of the Left sensor
        uint32_t    ExposureTimeLeft;

        /// \brief    Digital Gain of the Right sensor
        uint32_t    DigitalGainRight;

        /// \brief    Digital Gain of the Left sensor
        uint32_t    DigitalGainLeft;

        /// \brief    Analog Gain of the Right sensor
        uint32_t    AnalogGainRight;

        /// \brief    Analog Gain of the Left sensor
        uint32_t    AnalogGainLeft;


        /// \brief    Default constructor which reset all exposure parameters
        CExposureParams() :
            ExposureTimeRight(0),
            ExposureTimeLeft(0),
            DigitalGainRight(0),
            DigitalGainLeft(0),
            AnalogGainRight(0),
            AnalogGainLeft(0)
            {
            }
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    ROI (region of interest) for automatic sensor control  algorithm 
    ///////////////////////////////////////////////////////////////////////
    struct CSensorControlROIParams
    {
        /// \brief    If true then ROI is applied in automatic sensor control
        bool UseROI;

        /// \brief    Top left corner of ROI
        CPoint<2, unsigned int> ROITopLeft;

        /// \brief    Bottom right corner of ROI
        CPoint<2, unsigned int> ROIBottomRight;

        /// \brief    Default constructor, when ROITopLeft=ROIBottomRight=0 then ROI is not apply 
        CSensorControlROIParams() : UseROI(false), ROITopLeft(0, 0), ROIBottomRight(0, 0) {}
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    Sensor control parameters 
    ///////////////////////////////////////////////////////////////////////
    struct CSensorControlParams :public CExposureParams
    {
        
        /// \brief    If this flag is true then automatic algorithm for gain and exposure time adjustment (AGC) is activated. 
        bool AutoControl;

        /// \brief     ROI of each sensor
        CSensorControlROIParams Params[2];
        
        /// \brief    Default constructor, automatic sensor control is on 
        CSensorControlParams() : AutoControl(true){ }
    };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    Projector intensity levels
    ///////////////////////////////////////////////////////////////////////
    enum EProjectorLevel
    {
        eOff = 0,   ///< Turn projector off
        eLow,       ///< Low Projector intensity               
        eHigh       ///< High Projector intensity
     };

    ///////////////////////////////////////////////////////////////////////
    /// \brief    All projectors that can be assembled in InuSensor
    ///////////////////////////////////////////////////////////////////////
    enum EProjectors
    {
        ePatterns = 0,  ///< Main projector, Projecting Patterns 
        eFlood = 1,     ///< Projecting Flood  
        eNumOfProjectors
    };

    ////////////////////////////////////////////////////////////////////////
    /// \brief    Represents Inuitive's Sensor
    ///
    /// Role: Enables control of the sensor and generates all kinds of InuDev streams
    ///
    /// Responsibilities: 
    ///      1. Singleton object
    ///      2. Derives CBaseStream interface
    ///      3. Creates all kinds of InuDev streams
    ///      4. Provides information about InuSensor connection state 
    ///      5. Sensor control 
    ///      5. Get and Set different features of InuSensor
    ///
    ////////////////////////////////////////////////////////////////////////
    class CInuSensor 
    {
 
    public:

        enum  ESensorState
        {
            eUninitialized=0,
            eInitialized,
            eStarted,
            eSensorDisconnected,       /// Sensor is disconnected 
            eServiceNotConnected
        };

        enum  ESensorTemperatureType
        {
            eSensor1 = 0,  
            eSensor2
        };

        /// \brief    Prototype of callback function which is used by the Register method.
        ///
        /// This function is invoked any time a frame is ready, or if an error occurs. It provides information about InuSensor connection state
        /// The callback function arguments are: caller stream object, received Sensor state frame and result code.
        typedef std::function<void(std::shared_ptr<CInuSensor>, EConnectionState,  CInuError)> CallbackFunction;

        virtual ~CInuSensor() {} 

        /// \brief    Access to the Singleton object
        INUSTREAMS_API static std::shared_ptr<CInuSensor>  Create(const std::string& iTargetServiceId = std::string());

        // Generate all kinds of InuDev streams 
        virtual std::shared_ptr<CDepthStream> CreateDepthStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CVideoStream> CreateVideoStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CWebCamStream> CreateWebCamStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CImuStream>   CreateImuStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CGeneralPurposeStream> CreateGeneralPurposeStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CAudioStream>   CreateAudioStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CFeaturesTrackingStream>   CreateFeaturesTrackingStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CSlamStream>  CreateSlamStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CObjectsDetectionStream>  CreateObjectsDetectionStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CFaceRecognitionStream>  CreateFaceRecognitionStream(uint32_t iChannelID = 0) = 0;
        virtual std::shared_ptr<CCnnStream>  CreateCnnStream(uint32_t iChannelID = 0) = 0;
        /// \brief    Service initialization.
        /// 
        /// Invoked once before initialization of any other InuDev stream.
        /// After invoking Init method the sensor is still in low power consumption. 
        /// \param[in] iSensorParams    Initialized the sensor with these input parameters. It will be set to all assembled cameras.
        /// \param[in] iSensorID    Unique ID of the sensor that should be accessed. Tcp/ip address for remote sensor.  
        /// \return CInuError    Operation status which indicates on success or failure.
        virtual CInuError Init(const CSensorParams& iSensorParams = CSensorParams()) = 0;

        /// \brief    Start acquisition of frames.
        /// 
        /// Shall be invoked only after the service is successfully initialized and before any request
        /// for new frame (push or pull).
        /// \param[in] iUserID    Unique ID of the user who should be tracked.
        /// \return CInuError    Operation status. 
        virtual CInuError Start(const std::string& iUserID = std::string()) = 0;


        /// \brief    Service termination. 
        /// 
        /// Shall be invoked when the service is no longer in use and after frames acquisition has stopped.
        /// \return CInuError    Operation status which indicates success or failure. 
        virtual CInuError Terminate() = 0;

        /// \brief    Stop acquisition of frames. 
        /// 
        /// Shall be invoked after requests for frames are no longer sent and before service termination
        /// (only if Start() was invoked).
        /// \return CInuError    Operation status which indicates success or failure. 
        virtual CInuError Stop() = 0;



        /// \brief    Try to connect to Inuitive Sensor.
        /// 
        /// Communicate with InuService and try to connect to Inuitive Sensor.
        /// \return CInuError    Error description when connection fails.
        virtual CInuError Connect() = 0;

        /// \brief    Try to disconnect from Inuitive Sensor.
        /// 
        /// Stop Communicate to InuService.
        /// \return CInuError    Error description when disconnection fails.
        virtual CInuError Disconnect() = 0;

        /// \brief    Get the connection state of the sensor. 
        /// \return EConnectionState    
        virtual EConnectionState GetConnectionState() const = 0;

        /// \brief    Get the Sensor state. 
        /// \return ESensorState    
        virtual ESensorState GetState() const = 0;

        /// \brief    Get the Sensor Temperature. 
        /// \param[in] iType  Temperature sensor type.
        /// \param[out] oTemperature    returns the temperature in Celsius .
        /// \return CInuError    Error description of temperature reading.    
        virtual CInuError GetSensorTemperature(ESensorTemperatureType iType, float& oTemperature) = 0;
        
        /// \brief    Registration for receiving InuSensor state notifications (push). 
        /// 
        /// The provided callback function is called only when the sensor state is changed. 
        /// \param[in] iCallback    Callback function which is invoked whenever the sensor state is changed.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError Register(CallbackFunction iCallback) = 0;

        /// \brief		SW reset of InuSensor, it resets both SW and HW.
        /// \return CInuError If failed to reset the sensor 
        virtual CInuError Reset() = 0;
        
        /// \brief    Get current initialized  parameters. It should be used when other client application might be running and initialized InuSensor. 
        /// \param[out] oParams    Retrieved sensor parameters.
        /// \param[in] iCameraName    Parameters of which camera to retrieve, eAllCameras will return with error.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError  GetSensorParams(CSensorParams& oParams, ECameraName iCameraName) const = 0;

        /// \brief    Overwrite InuSensor initialized  parameters.
        /// 
        /// If this function is invoked after Start then only FPS will be changed and the new resolution will be ignored.
        /// \param[in] oParams    New parameters to apply.
        /// \param[in] iCameraName    To which camera to apply these parameters, if eAllCameras is used then it is applied to all.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError  SetSensorParams(const CSensorParams& iParams, ECameraName iCameraName) = 0;
        
        /// \brief    Get information about the SW and HW components. 
        /// \param[out] oVersion    Version description of each component.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError GetVersion(std::map<CEntityVersion::EEntitiesID, CEntityVersion>& oVersion) = 0;

        /// \brief    Get the configuration of InuSensor. 
        ///
        /// Should be called only if after the sensor was initiated. 
        /// \param[out] oConfiguration    returned configuration.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError GetSensorConfiguration(CSensorConfiguration& oConfiguration) const = 0;

        /// \brief    Get Sensor Control (AGC) data.
        ///
        /// It should be called only if after any Video related stream (e.g. Video, Depth, Head, etc.) was initiated. 
        /// \param[out] oParams    Sensor Control parameters.
        /// \param[in] iChannel    Which channel, it is possible to define different parameters to each channel of video stream.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError GetSensorControlParams(CSensorControlParams& oParams, uint32_t iChannel = 0) const = 0;

        /// \brief    Set Sensor Control (AGC) data. 
        /// 
        /// It should be called only if after any Video related stream (e.g. Video, Depth, Head, etc.) was initiated. 
        /// \param[in] iParams    New Sensor Control parameters.
        /// \param[in] iChannel    Which channel, it is possible to define different parameters to each channel of video stream.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError SetSensorControlParams(const CSensorControlParams& iParams, uint32_t iChannel = 0) const = 0;

        /// \brief    Load Registers from input file. 
        /// 
        /// It should be called only if after the sensor was initiated. 
        /// The file format should be provided by Inuitive technical staff.
        /// \param[in] iFileName    Input file name provided by Inuitive.
        /// \return CInuError    Error code, InDev::eOK if operation successfully completed.
        virtual CInuError LoadRegistersConfigurationFile(std::string iFileName) const = 0;


        /// \brief		Get Optical data information 
        /// \param[out]  oOpticalData   Output optical data information
        /// \param[int  iCameraName   Optical data of which camera 
        /// \param[int  iTemperature  Optical data of which temperature, in case of default the optical data of active calibration is returned 
        /// \return CInuError   If InuSensor isn't initialized then eStateError is returned 
        virtual CInuError GetOpticalData(
            COpticalData& oOpticalData, 
            ECameraName iCameraName = eVideo, 
            int iTemperature = std::numeric_limits<int>::max()) const  = 0;

        /// \brief		Set one of the assembled projectors' state
        /// 
        /// \param[in]  iLevel : High - high power, Low low power, Off - projector off 
        /// \param[in]  EProjectors - Projector name, eNumOfProjectors is illegal value
        /// \return CInuError
        virtual CInuError SetProjectorLevel(EProjectorLevel iLevel, EProjectors iProjectorID) const = 0;

        /// \brief		Get one of the assembled projectors' state
        /// 
        /// \param[out]  iLevel : High - high power, Low low power, Off - projector off 
        /// \param[out]  EProjectors - Projector name, eNumOfProjectors is illegal value
        /// \return CInuError
        virtual CInuError GetProjectorLevel(EProjectorLevel& iLevel, EProjectors iProjectorID) const = 0;

        /// \brief Record NU3000 streams
        /// \param[in]  iDestinationDirectory Destination directory for recording output. Send empty string to stop recording.
        /// \param[in]  iTemplateName string which will be concatenate to output file name.
        /// \param[in]  iDuration recording time in ms.
        /// \return CInuError eOK indicates that request was successfully processed but it doesn't indicate that recording is completed
        virtual CInuError  Record(const std::string& iDestinationDirectory,
            const std::string& iTemplateName = std::string(),
            uint64_t iDuration = CBaseStream::RECORD_INFINITE) const = 0;

        /// \brief Record NU3000 streams
        /// \param[in]  iDestinationDirectory Destination directory for recording output. Send empty string to stop recording.
        /// \param[in]  iTemplateName string which will be concatenate to output file name.
        /// \return CInuError eOK indicates that request was successfully processed but it doesn't indicate that recording is completed
        virtual CInuError  Snapshot(const std::string& iDestinationDirectory,
            const std::string& iTemplateName = std::string(),
            uint64_t iFileNameIndex = CBaseStream::RECORD_INFINITE) const = 0;
    protected:

        CInuSensor() {}
    };

}

#endif  // __INUSENSOR_H__
