#pragma once

#include <vector>
#include <string>
#include <thread>

// #include <opencv2/core.hpp>

#include "internal/DepthCameraImpl.h"
//#include "internal/DepthCameraSettingsImpl.h"

namespace b3dd {

// This DepthCamera corresponds to the SPRD implementation
// It is implemented in Java

class DepthCamera_Q845 : public DepthCameraImpl {

public:
    DepthCamera_Q845();
    virtual ~DepthCamera_Q845();

	/**
	* Detect if a depth camera of this type is detected
	* @return True if a depth camera is detected
	*/
	static bool detectDevice();

    virtual void connect();

    virtual void disconnect();

	virtual DepthCameraError connectSync() {
		return DepthCameraError();
	}

	virtual DepthCameraError openSync() {
		return DepthCameraError();
	}

	virtual DepthCameraError closeSync() {
		return DepthCameraError();
	}

    DepthCameraError startStreamSync();

    DepthCameraError stopStreamSync();

    /**
    * Implemented in Java
    */
    virtual void open();

    /**
    * Implemented in Java
    */
    virtual void close();

    // Should be a fast operation to start streaming
    // Matches "stopStream"
    virtual DepthCameraError startStream() {};

    // Should be a fast operation to stop streaming
    // Matches "startStream"
    virtual DepthCameraError stopStream() {};

    /**
    * Java DepthCamera class call this function to pass in camera frames
    */
    virtual void onFrame(DepthCameraFramePtr framePtr);

    virtual void resetFrameTime() {}


    static void doStreaming(DepthCamera* depthCamera);

    std::shared_ptr<std::thread> _doStreamingThreadPtr;  /** streaming worker thread pointer. */


private:

    static const std::string TAG;  // TODO  rename to TAG

};
using DepthCamera_Q845Ptr = std::shared_ptr<DepthCamera_Q845>;

} // namespace b3d
