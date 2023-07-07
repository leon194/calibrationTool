/*M///////////////////////////////////////////////////////////////////////////////////////
//
// DepthCameraState is the base state class for all DepthCamera states
//
// Copyright (c) 2017 Bellus3D, Inc. All rights reserved.
//
// 7/12/2017    jingliu created
//
//M*/

#pragma once

#include <memory>

#include "B3D4ExportDef.h"
#include "B3DCameraError.h"

namespace b3d4 {

/** Forward declaration of DepthCamera */  // otherwise DepthCamera is not usable in this file
class B3DCamera;

/** 
* @brief  An abstract base class for representing DepthCamera states.
*
* Detailed descriptions go here...
*/ 
class DLLEXPORT B3DCameraState {
public:

    /**
    * An enum that represents the DepthCamera state
    * More detailed enum description.
    */
	enum class StateType {
		DISCONNECTED,
		CONNECTING,
		DISCONNECTING,  // <- never used yet?
		CONNECTED,
		OPENING,
		CLOSING,
		OPEN,
		STARTING,
		STOPPING,
        STREAMING
	};

	virtual ~B3DCameraState() {};


	/**
	* @brief  Get state type of current DepthCameraState
	*
	* @return b3d::DepthCameraState::StateType
	*/
	virtual StateType getStateType() const = 0;

protected:
	B3DCameraState() {};
};
using DepthCameraStatePtr = std::shared_ptr<B3DCameraState>;


class DepthCameraDisconnectedState : public B3DCameraState {
public:
	DepthCameraDisconnectedState() {
		// nothing
	}

	virtual StateType getStateType() const {
		return B3DCameraState::StateType::DISCONNECTED;
	}
};

class DepthCameraConnectingState : public B3DCameraState {
public:
    DepthCameraConnectingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::CONNECTING;
    }
};

class DepthCameraDisconnectingState : public B3DCameraState {
public:
    DepthCameraDisconnectingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::DISCONNECTING;
    }
};

class DepthCameraConnectedState : public B3DCameraState {
public:
    DepthCameraConnectedState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::CONNECTED;
    }
};

class DepthCameraOpeningState : public B3DCameraState {
public:
    DepthCameraOpeningState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::OPENING;
    }
};

class DepthCameraClosingState : public B3DCameraState {
public:
    DepthCameraClosingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::CLOSING;
    }
};

class DepthCameraOpenState : public B3DCameraState {
public:
    DepthCameraOpenState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::OPEN;
    }
};

class DepthCameraStartingState : public B3DCameraState {
public:
    DepthCameraStartingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::STARTING;
    }
};

class DepthCameraStoppingState : public B3DCameraState {
public:
    DepthCameraStoppingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::STOPPING;
    }
};

class DepthCameraStreamingState : public B3DCameraState {
public:
    DepthCameraStreamingState() {
        // nothing
    }

    virtual StateType getStateType() const {
        return B3DCameraState::StateType::STREAMING;
    }
};


/**
* @brief  An abstract base class for observers who want to receive DepthCameraState updates
*
* Detailed descriptions go here...
*/
class B3DCameraObserver {
public:

	/**
	* @brief  Report DepthCameraState updates
	* 
	* @param[in] depthCameraState
	*/
    virtual void onUpdate(const B3DCameraState& depthCameraState, B3DCamera* depthCameraPtr) = 0;
	
    /**
    * @brief  Report errors from DepthCamera
    *
    * @param[in] const B3DError & b3DError
    * @param[in] DepthCamera * depthCameraPtr
    * @return void
    */
    virtual void onError(const B3DCameraError& depthCameraError, B3DCamera* depthCameraPtr) = 0;
};
using B3DCameraObserverPtr = std::shared_ptr<B3DCameraObserver>;


} // namespace b3d4