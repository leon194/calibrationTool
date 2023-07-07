#pragma once

#include <set>
#include <queue>
#include <mutex>  // std::mutex, std::lock_guard


#include "B3DCamera.h"
#include "B3DCameraStreamListener.h"

namespace b3d4 {


class B3DCameraImpl : public B3DCamera {
public:

    //  --------  Override public functions, implemented in current class  --------  //

    virtual void registerStreamListener(B3DCameraStreamListenerPtr listener) final;

    virtual void unregisterStreamListener(B3DCameraStreamListenerPtr listener) final;

    virtual void registerObserver(B3DCameraObserverPtr observer) final;

    virtual void unregisterObserver(B3DCameraObserverPtr observer) final;

    virtual void setStreamTypes(const std::set<B3DCameraFrame::FrameType> frameTypes) final;

    virtual B3DCameraState::StateType getCurrentStateType() const final;

    virtual std::string getDepthStatsString() const final;


    //  --------  Override public functions, should be implemented by Derived classes --------  //

    virtual B3DCameraError connectSync() override = 0;

    virtual B3DCameraError openSync() override = 0;

    virtual B3DCameraError closeSync() override = 0;

    virtual B3DCameraError startStreamSync() override = 0;

    virtual B3DCameraError stopStreamSync() override = 0;

    virtual std::string getDeviceName() const override { return "N/A"; }

    virtual std::string getDeviceID() const override { return "N/A"; }

    virtual std::string getFolderPath() override = 0;

    virtual B3DCameraError setProjectorLevel(ProjectorLevel level) override { return B3DCameraError(); }


    //  --------  Interfaces NOT exposed to developers yet  --------  //
    
    struct RawFrame {
        RawFrame(int _x = -1, int _y = -1, int _width = -1, int _height = -1) {
            x = _x;
            y = _y;
            width = _width;
            height = _height;
        }

        int x;
        int y;
        int width;
        int height;
    };


    // Each device may have up to 3 camera
    enum CameraType {
        CAMERA_L = 0,  // IR left  camera (from camera view facing the world)
        CAMERA_R = 1,  // IR right camera
        CAMERA_M = 2,  // color    camera
        CAMERA_D = 3   // depth    camera
    };


    // This method is not exposed to developers
    virtual void setCurrentStateType(B3DCameraState::StateType currentStateType);

    // Return a string that represents current DepthCamera state type
    static std::string getDepthCameraStateString(B3DCameraState::StateType stateType);


    /**
    * Call this method to reset the DepthCamera frame time.
    * Subsequent frames will have timestamps relative to the time this method is called.
    */
    virtual void resetFrameTime() = 0;


    virtual std::set<B3DCameraFrame::FrameType> getStreamTypes() const;

    /**
    * For Android/iOS DepthCamera class call this function to pass in camera frames
    * Not all derived classes need to implement this method (e.g. B3D3)
    */
    virtual void onFrame(B3DCameraFramePtr framePtr) = 0;

    void cacheB3DCameraFrame(B3DCameraFramePtr framePtr);


    // virtual void parseByteDataToFrame() = 0;
    // void cacheByteData(B3DCameraFramePtr framePtr, const unsigned char* byteData);


    // Access settings of DepthCamera
    //virtual DepthCameraSettingsImplPtr getDepthCameraSettings() const;

    // Get depth camera sensor calibration data
    //virtual b3di::CameraParams getCameraParams(CameraType sensorType) const;

    // Manually set current depth camera calibration data
    // DepthCamera will read default calibration data (on flash) when open()
    //virtual void setCameraParams(const b3di::CameraParams& params, CameraType sensorType);

    // Not sure if this should be done via "setDepthCameraSettings"
    virtual B3DCameraError setExposureStereo(int microSeconds);

    static int getFrameStoreIndexFromFrameType(B3DCameraFrame::FrameType frameType);

protected:

    // Constructor
    B3DCameraImpl();
    virtual ~B3DCameraImpl() {};

    virtual void loadCalibrationData(const std::string& calibDataPath);

    virtual void updateDepthProcessorSettings();

    virtual void reportUpdateToAllObservers(const B3DCameraState& depthCameraState);

    virtual void reportErrorToAllObservers(const B3DCameraError& b3dError);

    // based on current streaming frames, return the indices for frame stores
    std::vector<int> getFrameIndicesFromFrameTypes() const;

    B3DCameraFrame::FrameType getFrameTypeFromFrameIndex(int frameIndex) const;

    // Check if current settings configured to stream input frame type
    bool isSetToStream(B3DCameraFrame::FrameType frameType);


    void addFrameTimestampToQueue(double currentFrameTime);

    double getCurrentFPS() const;

    /**
    * FIXME: should be removed
    * will throw error if not called at expectedState
    */
    virtual void checkDepthCameraState(B3DCameraState::StateType expectedState, const std::string& methodName);

    virtual B3DCameraError checkCameraState(B3DCameraState::StateType expectedState, const std::string& methodName);

    /**
    *  will report updates to observers
    */
    virtual void updateDepthCameraState(B3DCameraState::StateType nextState, bool isReporting = true);


    //  --------  Member variables of DepthCameraImpl  --------  //
    std::string _deviceID;    // unique ID for every camera device (e.g. LLL12345678)
    std::string _deviceName;  // the version name for current camera device (e.g. B3D2-EVT2.1)


    B3DCameraState::StateType _currentCameraState;

    //DepthCameraSettingsImplPtr _settingsPtr;

    std::set<B3DCameraFrame::FrameType> _currentFrameTypes;  // frame types in streaming

    std::vector<B3DCameraObserverPtr> _observers;

    B3DCameraStreamListenerPtr _streamListener;


    //  ----  Depth Processing related  ----  //
    //DepthProcessorExt _depthProcessor;

    bool _isDepthProcessorSettingsUpdated;
    //DepthMapSettingsImplPtr _depthMapSettingsPtr;  // DepthCamera depth interface
    //DepthConfigExtPtr _depthConfigPtr;  // depth processor config settings


    // calculate ms per compute depth frame takes
    double _computeDepthTime;

    // calculate FPS    
    const int TIMESTAMP_QUEUE_SIZE = 10;
    std::queue<double> _frameTimestamps;  // (ms) keep record of past N frames' timestamps, for FPS control

    std::mutex _cacheFramesMutex;

    const int CACHE_FRAME_SIZE = 2;
    std::queue<B3DCameraFramePtr> _cachedFrames;
};

using B3DCameraImplPtr = std::shared_ptr<B3DCameraImpl>;

} // namespace b3dd
