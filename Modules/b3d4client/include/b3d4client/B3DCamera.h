/*M///////////////////////////////////////////////////////////////////////////////////////
//
// DepthCamera is the main interface for depth camera
//
// Copyright (c) 2019 Bellus3D, Inc. All rights reserved.
//
// 3/05/2019    jingliu created
//
//M*/

#pragma once

#include <vector>
#include <set>

// B3D4 Client
#include "B3D4ExportDef.h"
#include "B3DCameraState.h"
#include "B3DCameraStreamListener.h"


namespace b3d4 {

/**
* @brief  An abstract base class for representing DepthCamera
*
* Detailed descriptions:
*/
class DLLEXPORT B3DCamera {

public:

    enum DCamType {
        B3D4 = 0,
        FILES = 2,
    };

    enum DecodeType {
        CALIBRATIONTOOL  = 0,
        FACEDETECTION    = 1,
        SINGLEVIEW       = 2,
        FACELANDMARK     = 3,
        RECALIBRATION    = 4,
        DEPTHCOMPUTATION = 5,
        DIAGNOSTIC       = 6
    };

    enum TripleCamResolution {
        RESOLUTION_2M = 0,
        RESOLUTION_8M = 1,
    };

    /**
    * @brief  This method should return a list of supported DepthCamera devices
    * (Experimental)
    * @return std::vector<std::string>
    */
    static std::vector<std::string> detectSupportedDevices();

    /**
    * @brief  Constructor for DepthCamera that returns a smart pointer to the DepthCamera instance.
    */
    static std::shared_ptr<B3DCamera> newB3DCamera(DCamType type = B3DCamera::B3D4);

    virtual ~B3DCamera() {};

    virtual B3DCameraError connectSync() = 0;

    virtual B3DCameraError openSync() = 0;

    virtual B3DCameraError closeSync() = 0;

    virtual B3DCameraError startStreamSync() = 0;

    virtual B3DCameraError stopStreamSync()  = 0;


    // DepthCamera stream listeners, not exposed to developers yet
    virtual void registerStreamListener(B3DCameraStreamListenerPtr listener) = 0;

    virtual void unregisterStreamListener(B3DCameraStreamListenerPtr listener) = 0;


    /**
    * @brief  Registers observers to receive DepthCameraState updates
    *
    * @param[in] DepthCameraObserverPtr observer
    * @return void
    *
    * @see DepthCameraObserver
    */
    virtual void registerObserver(B3DCameraObserverPtr observer) = 0;


    /**
    * @brief  Stop receiving DepthCameraState updates
    *
    * @param[in] DepthCameraObserverPtr observer
    * @return void
    */
    virtual void unregisterObserver(B3DCameraObserverPtr observer) = 0;


    /**
    * @brief  Configures which camera streams will be sent to listener
    *
    * @param[in] a set of FrameType(s)
    * @return void
    */
    virtual void setStreamTypes(const std::set<B3DCameraFrame::FrameType> frameTypes) = 0;


    /**
    * @brief  Get current state of DepthCamera
    * @return b3d::DepthCameraState::StateType An enum that represents the current state of DepthCamera.
    */
    virtual B3DCameraState::StateType getCurrentStateType() const = 0;

     
    /**
         * @brief  Returns FPS and compute depth time
         * @return a string containing the info
        */
    virtual std::string getDepthStatsString() = 0;

    /* *
         *    @brief  decode frames if needed, than will trigger listener->onFrame
         *
         * */
    virtual void decodeFrames(int8_t* bytearray,int64_t frameCount, int64_t timeStamp, int32_t mResolution, uint8_t isCapture, int col,int row) = 0;

    virtual void setDecodeType(DecodeType type) = 0;

    //virtual void setDepthMapSettings(DepthMapSettingsPtr settingsPtr) = 0;

    //virtual DepthConfigExtPtr getDepthProcessorSettings() = 0;

    //virtual void setDepthProcessorSettings(DepthConfigExtPtr depthConfigPtr) = 0;

    /**
    * restore the calibration files to factory values
    * depth camera should be connected before calling this method.
    * @return void
    */
    //virtual DepthCameraError restoreFactoryCalibrationData() = 0;


    /**
         * Get the scanning device ID, which is unique for every depth camera (e.g. LLL12345678)
         * return "" empty string if cannot find
         */
    virtual std::string getDeviceID() const = 0;


    /**
         * Get the scanning device name, which defines the depth camera hardware version (e.g. B3D2-EVT2.1)
         * return "" empty string if cannot find
         */
    virtual std::string getDeviceName() const = 0;

    /**
         * @brief  Returns where the calib.data depthCam.yml midCam.yml is
         * @return a string containing the path
        */
    virtual std::string getFolderPath() = 0;

    virtual void setCamPosition(std::string pos);

    virtual std::string getCamPosition();

    // For controlling IR projector
    enum ProjectorLevel {
        OFF = 0,
        LOW_POWER = 1,
        HIGH_POWER = 2
    };

    virtual B3DCameraError setProjectorLevel(ProjectorLevel level) = 0;

    DecodeType _decodeType;

    void setIsCancel(bool isCancel);

    bool getIsCancel();

protected:
    B3DCamera() {}


private:
    std::string _cameraPosition;
    std::string _dataFolder;
    bool _isCancel;
};

using B3DCameraPtr = std::shared_ptr<B3DCamera>;

} // namespace b3d4
