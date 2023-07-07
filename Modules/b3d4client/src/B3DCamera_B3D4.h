#pragma once

#include <vector>
#include <string>
#include <queue>


// B3D Camera
#include "B3DCameraImpl.h"

namespace b3d4 {

const std::string DIAGNOSTIC_FILE_PATH = "/sdcard/Bellus3d/Arc/ArcClient/diagnostic/";

class B3DCamera_B3D4 : public B3DCameraImpl {

public:
    B3DCamera_B3D4(DCamType type = B3DCamera::B3D4);

    virtual ~B3DCamera_B3D4();

	virtual B3DCameraError connectSync() override;

	virtual B3DCameraError openSync() override;

	virtual B3DCameraError closeSync() override;

	virtual B3DCameraError startStreamSync() override;

	virtual B3DCameraError stopStreamSync() override;

	virtual void decodeFrames(int8_t* bytearray,int64_t frameCount, int64_t timeStamp, int32_t mResolution, uint8_t isCapture, int col,int row);

    virtual void setDecodeType(DecodeType type);

    virtual void resetFrameTime() override;

    virtual void onFrame(B3DCameraFramePtr framePtr) override;

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
    cv::Mat diagnosticM, diagnosticL, diagnosticR;
};
using B3DCamera_B3D4Ptr = std::shared_ptr<B3DCamera_B3D4>;

} // namespace b3d4
