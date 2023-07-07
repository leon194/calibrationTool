package com.bellus3d.android.arc.b3d4client.camera;

/**
 * The <code>DepthCameraError</code> class represents possible error code from DepthCamera class.
 */
public class DepthCameraError {

    /**
     * Enum type that indicates which error is encountered.
     */
    public enum ErrorCode {
        B3D_NO_ERROR,            /**< Default is no error */
        FILE_IO_ERROR,           /**< Fails to read, write or delete a file or folder */
        FILE_MISSING,            /**< Cannot read a specified file */
        INVALID_STATE_ERROR,     /**< Methods called at invalid state: Call appropriate method first */
        CONNECTION_LOST,         /**< Camera disconnected: Plug in camera */
        NOT_CLOSED_PROPERLY,     /**< APP exits before closing camera properly */
        INTERNAL_LIBRARY_ERROR,  /**< Error occurred inside B3D internal library */
        SERVICE_NOT_RUNNING,      /**< InuService not running: Plug in camera to start service */
        INVALID_INPUT,
        PERMISSION_ERROR
    }

    public ErrorCode errorCode;
    public String debugMessage;

    /**
     * Creates a new DepthCameraError instance.
     * only called within B3D SDK
     */
    DepthCameraError() {
        errorCode    = ErrorCode.B3D_NO_ERROR;
        debugMessage = "";
    }

    /**
     * Creates a new DepthCameraError instance.
     * only called within B3D SDK
     */
    DepthCameraError(ErrorCode error, String message) {
        errorCode    = error;
        debugMessage = message;
    }
}
