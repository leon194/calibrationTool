package com.bellus3d.android.arc.b3d4client.camera;

/**
 * DepthCamera status
 */

public class DepthCameraState {

    public enum StateType {
        DISCONNECTED("disconnected"),
        CONNECTING("connecting"),
        DISCONNECTING("disconnecting"),  // < should be removed ?
        CONNECTED("connected"),
        OPENING("opening"),
        CLOSING("closing"),
        OPEN("open"),
        STARTING("starting"),
        STOPPING("stopping"),
        STREAMING("streaming");

        private String state;

        private StateType(String state) {
            this.state = state;
        }

        public String getName() {
            return this.state;
        }
    };

    DepthCameraState() {
        _stateType = StateType.DISCONNECTED;
    };

    private StateType _stateType;

    public StateType getStateType() { return _stateType;};

    public void setStateType(StateType stateType) { _stateType = stateType; };
}