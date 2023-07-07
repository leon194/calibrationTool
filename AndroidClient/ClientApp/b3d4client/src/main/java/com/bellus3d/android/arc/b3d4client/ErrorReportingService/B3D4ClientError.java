package com.bellus3d.android.arc.b3d4client.ErrorReportingService;

import android.os.Build;
import android.support.annotation.RequiresApi;

import java.util.HashMap;
import java.util.Map;

public class B3D4ClientError {

    /* use String value to switch language further */
    public Map<Integer, String> B3DErrorCategories_EN = new HashMap<>();
    public Map<Integer, String> B3DErrorMessages_EN = new HashMap<>();

    final private int SHIFT = 100;

    private int errorCode;

    public enum B3D_ERROR_CATEGORY
    {
        B3D_OK(0),
        B3D_FILE_IO_ERROR(1),
        B3D_CONNECTIVITY_ERROR(2),
        B3D_SENSOR_STREAMING_ERROR(3),
        B3D_DEPTH_COMPUTATION_ERROR(4),
        B3D_SINGLE_VIEW_MERGE_ERROR(5),
        B3D_SINGLE_CAMERA_BUILDMESH_ERROR(6),
        B3D_CONFIGURATION_ERROR(7),
        B3D_RECALIBRATION_ERROR(8),
        B3D_FINDLANDMARK_ERROR(9),
        B3D_OTHER_ERROR(130);

        private final int id;

        B3D_ERROR_CATEGORY(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_FILE_IO_ERROR
    {
        FILE_MISSING(101),
        FILE_READ_FAIL(102),
        FILE_WRITE_FAIL(103),
        FILE_FORMAT_NOT_SUPPORTED(104),
        FILE_TIMESTAMP_INVALID(105);

        private final int id;

        B3D_FILE_IO_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_CONNECTIVITY_ERROR
    {
        WIFI_OPEN_FAIL(201),
        HOST_DISCOVERY_FAIL(202),
        HOST_CONNECT_FAIL(203),
        INTERNET_ACCESS_SLOW(204);

        private final int id;

        B3D_CONNECTIVITY_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };



    public enum B3D_SENSOR_STREAMING_ERROR
    {
        SENSOR_STREAMING_OPEN_FAIL(301),
        SENSOR_STREAMING_CLOSE_FAIL(302),
        SENSOR_STREAMING_SET_EXPOSURE_TIME_FAIL(303),
        SENSOR_STREAMING_SWITCH_RESOLUTION_FAIL(304),
        SENSOR_CALL_AT_INVALID_STATE(305),
        SENSOR_IS_BUSY(306),
        SENSOR_IS_CANCELING(307),
        SENSOR_IS_PROCESSING(308),
        FLOOD_TURN_ON_FAIL(309),
        FLOOD_TURN_OFF_FAIL(310),
        PROJECTOR_TURN_ON_FAIL(311),
        PROJECTOR_TURN_OFF_FAIL(312),
        IMAGE_BUFFER_ID_INVALID(313),
        IMAGE_BUFFER_OVERFLOW(314),
        IMAGE_BUFFER_INVALID(315),
        ACTIVITY_CONTEXT_IS_NULL(316),
        CAPTURE_SETTING_IS_NULL(317);

        private final int id;

        B3D_SENSOR_STREAMING_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };


    public enum B3D_DEPTH_COMPUTATION_ERROR
    {
        DEPTH_INPUT_IMAGE_INVALID(401),
        DEPTH_INTPUT_CALIBATION_DATA_INVALID(402),
        DEPTH_INPUT_ROI_INVALID(403),
        DEPTH_INTPUT_RECT_MAP_INVALID(404),
        DEPTH_INPUT_CONFIG_INVALID(405);

        private final int id;

        B3D_DEPTH_COMPUTATION_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_SINGLE_VIEW_MERGE_ERROR
    {
        MERGE_SETTINGS_INVALID(501),
        MERGE_INPUT_DEPTH_INVALID(502),
        MERGE_INPUT_CALIBRATION_DATA_INVALID(503),
        MERGE_INPUT_NATIVE_MERGE_FAILED(504),
        MERGE_INPUT_CAPTURE_BUFFER_INVALID(505),
        MERGE_INPUT_NATIVE_CAPTURE_BUFFER_EXIST(506);

        private final int id;

        B3D_SINGLE_VIEW_MERGE_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_SINGLE_CAMERA_BUILDMESH_ERROR
    {
        MESH_SETTINGS_INVALID(601),
        MESH_INPUT_IMAGE_INVALID(602),
        MESH_INPUT_CALIBRATION_DATA_INVALID(603),
        MESH_FACE_TRACKING_FAIL(604),
        MESH_HEAD_POSE_TRACKING_FAIL(605),
        MESH_MERGE_DEPTH_FAIL(606),
        MESH_TEXTURE_COMPUTATION_FAIL(607);

        private final int id;

        B3D_SINGLE_CAMERA_BUILDMESH_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_CONFIGURATION_ERROR
    {
        CONFIG_HOTSPOT_MISSING(701),
        CONFIG_ADRRESS_MISSING(702),
        CONFIG_PORTS_MISSING(703),
        CONFIG_LAYOUT_MISSING(704);

        private final int id;

        B3D_CONFIGURATION_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_RECALIBRATION_ERROR
    {
        CALIB_NO_ERROR(801),
        INPUT_IMAGE_INVALID(802),
        CALIB_SIZE_INVALID(803),
        CALIB_NO_CHANGE(804);

        private final int id;

        B3D_RECALIBRATION_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_FIND_LANDMARK_ERROR
    {
        INIT_TRACKER_ERROR(901),
        FACE_NOT_FOUND(902),
        LANDMARKFILE_NOT_FOUND(903);

        private final int id;

        B3D_FIND_LANDMARK_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_BUFFER_SNAP_ERROR
    {
        CURRENT_RUNNING_COMMAND_NOT_SUPPORTED(1001),
        CURRENT_STREAM_NOT_RUNNING(1002);

        private final int id;

        B3D_BUFFER_SNAP_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_BUFFER_SNAPSHOT_RECEIVE_ERROR
    {
        CURRENT_BUFFER_CACHE_EMPTY(1101);

        private final int id;

        B3D_BUFFER_SNAPSHOT_RECEIVE_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_OPENCV_ERROR
    {
        OPENCV_ERROR(1201);

        private final int id;

        B3D_OPENCV_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_BUFFER_FETCH_ERROR
    {
        FETCH_INPUT_CAPTURE_BUFFER_INVALID(1301);

        private final int id;

        B3D_BUFFER_FETCH_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public enum B3D_OTHER_ERROR
    {
        OTHER_ERROR(13001),
        INTERNAL_IMPLEMENTATION_ERROR(13002),
        INVALID_TEXTURE_INPUT(13003),
        ANDROID_API_ERROR(13004),
        WRONG_DEPTHCAMERA_OPERATION(13005);

        private final int id;

        B3D_OTHER_ERROR(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public void setErrorCode(int errorCode) {
        this.errorCode = errorCode;
    }

    public int getErrorCode() { return errorCode; }

    @RequiresApi(api = Build.VERSION_CODES.N)
    public void setErrorMessage(int errorCode, String msg) {
        B3DErrorMessages_EN.replace(errorCode,msg);
    }

    public B3D4ClientError() {
        InitErrorMap();
        this.errorCode = B3D_ERROR_CATEGORY.B3D_OK.getValue();
    }

    public B3D4ClientError( int errorCode) {
        InitErrorMap();
        this.errorCode = errorCode;
    }

    public B3D4ClientError( int errorCode, String msg) {
        InitErrorMap();
        this.errorCode = errorCode;
        B3DErrorCategories_EN.put(errorCode,msg);
    }

    public String getErrorCategories(int error) {
        if(0 == error ) return B3DErrorCategories_EN.get(error);
        else return (B3DErrorCategories_EN.get((error/SHIFT)) == null) ?
                "Invalid Error Code" : B3DErrorCategories_EN.get((error/SHIFT));
    }

    public String getErrorMessage(int error) {
        return (B3DErrorMessages_EN.get((error)) == null) ?
                "Invalid Error Code" : B3DErrorMessages_EN.get((error));
    }

    void InitErrorMap() {

        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_OK.getValue(),"B3D_OK");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_FILE_IO_ERROR.getValue(),"B3D_FILE_IO_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_CONNECTIVITY_ERROR.getValue(),"B3D_CONNECTIVITY_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_SENSOR_STREAMING_ERROR.getValue(),"B3D_SENSOR_STREAMING_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_DEPTH_COMPUTATION_ERROR.getValue(),"B3D_DEPTH_COMPUTATION_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_SINGLE_VIEW_MERGE_ERROR.getValue(),"B3D_SINGLE_VIEW_MERGE_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_SINGLE_CAMERA_BUILDMESH_ERROR.getValue(),"B3D_SINGLE_CAMERA_BUILDMESH_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_CONFIGURATION_ERROR.getValue(),"B3D_CONFIGURATION_ERROR");
        B3DErrorCategories_EN.put(B3D_ERROR_CATEGORY.B3D_OTHER_ERROR.getValue(),"B3D_OTHER_ERROR");

        B3DErrorMessages_EN.put(B3D_ERROR_CATEGORY.B3D_OK.getValue(),"B3D OK");
        // B3D_FILE_IO_ERROR messages
        B3DErrorMessages_EN.put(B3D_FILE_IO_ERROR.FILE_MISSING.getValue(),"Failed to find file");
        B3DErrorMessages_EN.put(B3D_FILE_IO_ERROR.FILE_READ_FAIL.getValue(),"Failed to read file");
        B3DErrorMessages_EN.put(B3D_FILE_IO_ERROR.FILE_WRITE_FAIL.getValue(),"Failed to write file");
        B3DErrorMessages_EN.put(B3D_FILE_IO_ERROR.FILE_FORMAT_NOT_SUPPORTED.getValue(),"File format not support");
        B3DErrorMessages_EN.put(B3D_FILE_IO_ERROR.FILE_TIMESTAMP_INVALID.getValue(),"Session folder timeStamp not in correct order");

        // B3D_CONNECTIVITY_ERROR messages
        B3DErrorMessages_EN.put(B3D_CONNECTIVITY_ERROR.WIFI_OPEN_FAIL.getValue(),"Failed to open wifi");
        B3DErrorMessages_EN.put(B3D_CONNECTIVITY_ERROR.HOST_DISCOVERY_FAIL.getValue(),"Failed to discover host device");
        B3DErrorMessages_EN.put(B3D_CONNECTIVITY_ERROR.HOST_CONNECT_FAIL.getValue(),"Failed to connect host device");
        B3DErrorMessages_EN.put(B3D_CONNECTIVITY_ERROR.INTERNET_ACCESS_SLOW.getValue(),"Internet access is slow");

        // B3D_SENSOR_STREAMING_ERROR messages
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_STREAMING_OPEN_FAIL.getValue(),"Failed to open sensor streaming");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_STREAMING_CLOSE_FAIL.getValue(),"Failed to close sensor streaming");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_STREAMING_SET_EXPOSURE_TIME_FAIL.getValue(),"Failed to set sensor exposure time");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_STREAMING_SWITCH_RESOLUTION_FAIL.getValue(),"Failed to switch sensor streaming resolution");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_CALL_AT_INVALID_STATE.getValue(),"Camera Sensor State Error");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_IS_BUSY.getValue(),"Camera Sensor is Busy");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_IS_CANCELING.getValue(),"Camera Sensor is Canceling previous processing");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.SENSOR_IS_PROCESSING.getValue(),"Camera Sensor is processing, only accept cancel process");

        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.FLOOD_TURN_ON_FAIL.getValue(),"Failed to turn on IR flood");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.FLOOD_TURN_OFF_FAIL.getValue(),"Failed to turn off IR flood");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.PROJECTOR_TURN_ON_FAIL.getValue(),"Failed to turn on projector");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.PROJECTOR_TURN_OFF_FAIL.getValue(),"Failed to turn off projector");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.IMAGE_BUFFER_ID_INVALID.getValue(),"Image buffer id invalid");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.IMAGE_BUFFER_OVERFLOW.getValue(),"Image buffer overflow");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.IMAGE_BUFFER_INVALID.getValue(),"Image buffer invalid");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.ACTIVITY_CONTEXT_IS_NULL.getValue(),"Activity context is null");
        B3DErrorMessages_EN.put(B3D_SENSOR_STREAMING_ERROR.CAPTURE_SETTING_IS_NULL.getValue(),"capture setting is null");

        // B3D_DEPTH_COMPUTATION_ERROR messages
        B3DErrorMessages_EN.put(B3D_DEPTH_COMPUTATION_ERROR.DEPTH_INPUT_IMAGE_INVALID.getValue(),"Invalid input image for depth computation");
        B3DErrorMessages_EN.put(B3D_DEPTH_COMPUTATION_ERROR.DEPTH_INTPUT_CALIBATION_DATA_INVALID.getValue(),"Invalid calibration data for depth computation");
        B3DErrorMessages_EN.put(B3D_DEPTH_COMPUTATION_ERROR.DEPTH_INPUT_ROI_INVALID.getValue(),"Invalid ROI for depth computation");
        B3DErrorMessages_EN.put(B3D_DEPTH_COMPUTATION_ERROR.DEPTH_INTPUT_RECT_MAP_INVALID.getValue(),"Invalid RECT Map for depth computation");
        B3DErrorMessages_EN.put(B3D_DEPTH_COMPUTATION_ERROR.DEPTH_INPUT_CONFIG_INVALID.getValue(),"Invalid Config for depth computation");

        // B3D_SINGLE_VIEW_MERGE_ERROR messages
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_SETTINGS_INVALID.getValue(),"Invalid settings for single view merge");
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_INPUT_DEPTH_INVALID.getValue(),"Invalid input depth for single view merge");
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_INPUT_CALIBRATION_DATA_INVALID.getValue(),"Invalid calibration data for single view merge");
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_INPUT_NATIVE_MERGE_FAILED.getValue(),"Native single view merge failed");
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_INPUT_CAPTURE_BUFFER_INVALID.getValue(),"Invalid capture buffer for single view merge");
        B3DErrorMessages_EN.put(B3D_SINGLE_VIEW_MERGE_ERROR.MERGE_INPUT_NATIVE_CAPTURE_BUFFER_EXIST.getValue(),"Input capture buffer had exist at native side");

        // B3D_SINGLE_CAMERA_BUILDMESH_ERROR
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_SETTINGS_INVALID.getValue(),"Invalid settings for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_INPUT_IMAGE_INVALID.getValue(),"Invalid input images for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_INPUT_CALIBRATION_DATA_INVALID.getValue(),"Invalid input calibration data for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_FACE_TRACKING_FAIL.getValue(),"Failed to detect face for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_HEAD_POSE_TRACKING_FAIL.getValue(),"Failed to track head pose for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_MERGE_DEPTH_FAIL.getValue(),"Failed to merge depth for single camera buildmesh");
        B3DErrorMessages_EN.put(B3D_SINGLE_CAMERA_BUILDMESH_ERROR.MESH_TEXTURE_COMPUTATION_FAIL.getValue(),"Failed to compute texture for single camera buildmesh");

        // B3D_CONFIGURATION_ERROR messages
        B3DErrorMessages_EN.put(B3D_CONFIGURATION_ERROR.CONFIG_HOTSPOT_MISSING.getValue(),"Missing hotspot configuration");
        B3DErrorMessages_EN.put(B3D_CONFIGURATION_ERROR.CONFIG_ADRRESS_MISSING.getValue(),"Missing IP address configuration");
        B3DErrorMessages_EN.put(B3D_CONFIGURATION_ERROR.CONFIG_PORTS_MISSING.getValue(),"Missing network port configuration");
        B3DErrorMessages_EN.put(B3D_CONFIGURATION_ERROR.CONFIG_LAYOUT_MISSING.getValue(),"Missing camera layout configuration");

        // B3D_RECALIBRATION_ERROR messages
        B3DErrorMessages_EN.put(B3D_RECALIBRATION_ERROR.CALIB_NO_ERROR.getValue(),"Recalibration no error");
        B3DErrorMessages_EN.put(B3D_RECALIBRATION_ERROR.CALIB_SIZE_INVALID.getValue(),"Recalibration input image size invalid");
        B3DErrorMessages_EN.put(B3D_RECALIBRATION_ERROR.INPUT_IMAGE_INVALID.getValue(),"Recalibration input image empty");
        B3DErrorMessages_EN.put(B3D_RECALIBRATION_ERROR.CALIB_NO_CHANGE.getValue(),"Recalibration no need to change");

        // B3D_FINDLANDMARK_ERROR messages
        B3DErrorMessages_EN.put(B3D_FIND_LANDMARK_ERROR.INIT_TRACKER_ERROR.getValue(),"FindLandMark init tracker error, no face training data");
        B3DErrorMessages_EN.put(B3D_FIND_LANDMARK_ERROR.FACE_NOT_FOUND.getValue(),"FindLandMark can't find face");
        B3DErrorMessages_EN.put(B3D_FIND_LANDMARK_ERROR.LANDMARKFILE_NOT_FOUND.getValue(),"FindLandMark can't find file");

        // B3D_BUFFER_SNAP_ERROR messages
        B3DErrorMessages_EN.put(B3D_BUFFER_SNAP_ERROR.CURRENT_RUNNING_COMMAND_NOT_SUPPORTED.getValue(),"current running command not supported");
        B3DErrorMessages_EN.put(B3D_BUFFER_SNAP_ERROR.CURRENT_STREAM_NOT_RUNNING.getValue(),"current no stream running");

        // B3D_BUFFER_SNAPSHOT_RECEIVE_ERROR message
        B3DErrorMessages_EN.put(B3D_BUFFER_SNAPSHOT_RECEIVE_ERROR.CURRENT_BUFFER_CACHE_EMPTY.getValue(),"current buffer cache empty");

        // B3D_OPENCV_ERROR message
        B3DErrorMessages_EN.put(B3D_OPENCV_ERROR.OPENCV_ERROR.getValue(),"native opencv error");

        // B3D_BUFFER_FETCH_ERROR message
        B3DErrorMessages_EN.put(B3D_BUFFER_FETCH_ERROR.FETCH_INPUT_CAPTURE_BUFFER_INVALID.getValue(),"Invalid capture buffer for buffer fetch");

        // B3D_CAMERA_OTHER_ERROR
        B3DErrorMessages_EN.put(B3D_OTHER_ERROR.OTHER_ERROR.getValue(),"Other Error");
        B3DErrorMessages_EN.put(B3D_OTHER_ERROR.INTERNAL_IMPLEMENTATION_ERROR.getValue(),"Function not implement");
        B3DErrorMessages_EN.put(B3D_OTHER_ERROR.INVALID_TEXTURE_INPUT.getValue(),"Input Texture Invalid");
        B3DErrorMessages_EN.put(B3D_OTHER_ERROR.ANDROID_API_ERROR.getValue(),"Android API Error");
        B3DErrorMessages_EN.put(B3D_OTHER_ERROR.WRONG_DEPTHCAMERA_OPERATION.getValue(),"Operation command in Wrong DepthCamera Type");
    }

}
