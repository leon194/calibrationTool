package com.bellus3d.android.arc.b3d4client;

public class AppStatusManager {

    private static AppStatusManager instance = new AppStatusManager();

    public enum AppState {
        CAMERA_STARED,
        CAMERA_STAR_FAILED,
        WIFI_OFF, //reserved
        WIFI_ON, //reserved
        WIFI_DISCONNECTED, //reserved
        WIFI_SCANNING, //reserved
        WIFI_SCANNED, //reserved
        WIFI_CONFIGURED, //reserved
        WEBSOCKET_DISCONNECTED,
        WEBSOCKET_CONNECTION_FAILED,
        WEBSOCKET_READY_TO_CONNECT,
        WEBSOCKET_CONNECTING,
        WEBSOCKET_CONNECTED,
        CAMERA_RECEIVED_COMMAND,
        CAMERA_RECORDING,
        CAMERA_RECORDING_ERROR,
        CAMERA_RECORDING_FINISH, //reserved
        CAMERA_PROCESSING_OR_TRANSFER,
        CAMERA_PROCESSING_ERROR,
        CAMERA_PROCESSING_COMPLETED
    }

    private AppState state;
    private StatusLightService statusLightService;

    public static AppStatusManager getInstance() {
        return instance;
    }

    private AppStatusManager() {
        statusLightService = new StatusLightService();
    }

    public AppState getState() {
        return state;
    }

    public void setState(AppState state) {
        this.state = state;
        switch (state){
            case CAMERA_STARED:
            case WEBSOCKET_CONNECTION_FAILED:
            case WEBSOCKET_DISCONNECTED:
                statusLightService.setLight(StatusLightService.Light.YELLOW);
                break;
            case CAMERA_STAR_FAILED:
            case CAMERA_RECORDING_ERROR:
            case CAMERA_PROCESSING_ERROR:
                statusLightService.setLight(StatusLightService.Light.RED);
                break;
            case WEBSOCKET_CONNECTING:
            case WEBSOCKET_CONNECTED:
            case CAMERA_PROCESSING_COMPLETED:
            case CAMERA_RECEIVED_COMMAND:
            case CAMERA_RECORDING_FINISH:
                statusLightService.setLight(StatusLightService.Light.GREEN);
                break;
            case CAMERA_RECORDING:
                statusLightService.setLight(StatusLightService.Light.WHITE);
                break;
            case CAMERA_PROCESSING_OR_TRANSFER:
                statusLightService.setBlinkLight(StatusLightService.Light.BLUE);
                break;
        }
    }

    public boolean passedState(AppState state){
        return getState().ordinal() >= state.ordinal();
    }
}
