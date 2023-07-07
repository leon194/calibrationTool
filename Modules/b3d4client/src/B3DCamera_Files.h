#pragma once

#include <vector>
#include <string>
#include <queue>


// B3D Camera
#include "B3DCameraImpl.h"


// B3D Util
//#include "TProfile.h"
#include <b3di/B3d_API.h>
#include <b3di/FileUtils.h>
#include <b3di/depthtoobj.h>
#include <b3di/FrameStore.h>

namespace b3d4 {

// This DepthCamera corresponds to the Inuitive chip based implementation

static std::vector<std::string> demo_campath {
    "/sdcard/B3D4/SampleDatasets/20190507_l2/", //l2 angle_05
    "/sdcard/B3D4/SampleDatasets/20190507_l1/", //l1 angle_03
    "/sdcard/B3D4/SampleDatasets/20190507_c/",  //c  angle_01
    "/sdcard/B3D4/SampleDatasets/20190507_r1/", //r1 angle_02
    "/sdcard/B3D4/SampleDatasets/20190507_r2/"  //r2 angle_04
    "/sdcard/B3D4/SampleDatasets/20190507_d1/"  //d1 angle_05
    "/sdcard/B3D4/SampleDatasets/20190507_u1/"  //u1 angle_06
};

class B3DCamera_Files : public B3DCameraImpl {

public:
	B3DCamera_Files(DCamType type = B3DCamera::FILES);
    virtual ~B3DCamera_Files();

	virtual B3DCameraError connectSync() override;

	virtual B3DCameraError openSync() override;

	virtual B3DCameraError closeSync() override;

	virtual B3DCameraError startStreamSync() override;

	virtual B3DCameraError stopStreamSync() override;

	virtual void decodeFrames(int8_t* bytearray,int64_t frameCount, int64_t timeStamp, int32_t mResolution, uint8_t isCapture, int col,int row);

    virtual void setDecodeType(DecodeType type);

    virtual void resetFrameTime() override;

    virtual void onFrame(B3DCameraFramePtr framePtr) override;

    // Not sure if this should be done via "setDepthCameraSettings"

    virtual B3DCameraError setProjectorLevel(ProjectorLevel level) override;

    /**
    * Detect if a depth camera of this type is detected
    * @return True if a depth camera is detected
    */
    static bool detectDevice();

    virtual std::string getDepthStatsString();

    virtual std::string getDeviceID();
    virtual std::string getDeviceName();
    virtual std::string getFolderPath() override;
	std::string _datadir;
    b3di::CameraParams depthCam;                // input depth cam params
    b3di::CameraParams colorCam;                // input color cam params
    b3di::FrameStore framesL, framesR, framesRGB, framesD;

protected:

    // release resources when DepthCamera is disconnected
    virtual void dispose();

    // Load files from camera board (device specific, dependent on deviceId)
    // 1. Device Name (e.g. B3D2-EVT2.1 _ high or low power?)
    // 2. MCU_Version / Device Version (e.g. B3D2_20170222)
    // 3. Calibration Data
    void loadDeviceInfo(const std::string& deviceId);

    // Load default calibration files from device flash
    void loadCalibrationFiles();

private:

    static void printTiming(double totalTime, const std::string& operationMessage);

    static void printPerFrameInfo(const std::string& perFrameMessage);

    void addFrameToQueue(double currentFrameTime);


private:

    static const std::string TAG;  // TODO  rename to TAG

    DCamType _type;

    bool _streamRGB;

    std::queue<double> _previousFramesTimestamps;  // (ms) keep record of past N frames' timestamps, for FPS control
    bool _resetDepthProfile;
    double _currentDepthCompuTime;

};

using B3DCamera_FILESPtr = std::shared_ptr<B3DCamera_Files>;

} // namespace b3d4
