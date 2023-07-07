package com.bellus3d.android.arc.b3d4client;

import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.support.annotation.RequiresApi;
import android.util.Pair;

import com.bellus3d.android.arc.b3d4client.ConfigService.LocalConfigService;
import com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError;
import com.bellus3d.android.arc.b3d4client.JsonModel.BufferIdResponseTo;
import com.bellus3d.android.arc.b3d4client.JsonModel.StreamCaptureRequest;
import com.bellus3d.android.arc.b3d4client.JsonModel.ResponseTo;
import com.bellus3d.android.arc.b3d4client.JsonModel.StreamDepthNotification;
import com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel.DeviceConfig;
import com.bellus3d.android.arc.b3d4client.JsonModel.MergeRequest;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;
import com.bellus3d.android.arc.b3d4client.NetworkService.NetworkService;
import com.bellus3d.android.arc.b3d4client.camera.B3D4NativeProcessorListener;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.bellus3d.android.arc.b3d4client.camera.DepthMapSettings;
import com.google.gson.Gson;

import java.io.File;
import java.io.IOException;
import java.util.Vector;

import static com.bellus3d.android.arc.b3d4client.AppStatusManager.AppState.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_CONFIGURATION_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.ErrorReportingService.B3D4ClientError.B3D_FILE_IO_ERROR.*;
import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.*;

public class ProcessingService {
    private DepthMapSettings _depthMapSettings;
    private B3DCaptureSettings _captureSettings;
    private StreamHelper _depthFrameStatistics = null;
    private NetworkService _networkService;
    DeviceConfig _deviceConfig;

    private B3D4NativeProcessorListener _registrantListener;
    private B3D4ClientError _b3D4ClientError;
    private Gson gson;

    private float _calibDispErr = -1.0f;
    private int _calibErrCode = 0;

    private boolean _isCancelProcessing = false;

    /* CameraStreamProcessor::ProcessType */
    public enum PROCESS_TYPE
    {
        PROCESS_TYPE_FACE(0),
        PROCESS_TYPE_SINGLEVIEW(1),
        PROCESS_TYPE_FACELANDMARK(2),
        PROCESS_TYPE_RECALIBRATION(3),
        PROCESS_TYPE_DEPTHCOMPUTE(4),
        PROCESS_TYPE_DIAGNOSTIC(5);

        private final int id;

        PROCESS_TYPE(int id) { this.id = id; }

        public int getValue() { return id; }
    };

    public ProcessingService() {
        _b3D4ClientError = new B3D4ClientError();
        gson = new Gson();
    }

    public void setNetworkService(NetworkService networkService){
        _networkService = networkService;
    }

    public void setDeviceConfig(DeviceConfig deviceConfig) {
        _deviceConfig = deviceConfig;
    }

    /* set current capture setting to ProcessingService */
    public void setProcessingSettings(B3DCaptureSettings captureSettings){
        _captureSettings = captureSettings;
    }

    public void setStreamHelper(StreamHelper streamStatistics){
        _depthFrameStatistics = streamStatistics;
    }

    public void registerNativeProcessorListener(B3D4NativeProcessorListener listener){
        _registrantListener = listener;
    }

    /* reserve for calibrationTool and RatestTool */
    public void addFrameToNative(FrameBuffer processBuffer) {
    }

    public void InitProcessingParemeter(int processType) {
        InitProcessingParemeterJNI(processType);
    }


    public void startProcessing(MMFrameBuffer processBufferV, int mResolution){
        /* prepare for merge */
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                preparefolder(_captureSettings.getMergeBufferID(), _captureSettings.isDebug());
            }
        });

        _calibDispErr = -1.0f;
        _calibErrCode = 0;
        _isCancelProcessing = false;

        InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_SINGLEVIEW.getValue());

        LocalConfigService.updateSaveResultFlagJNI(_captureSettings.isDebug());

        String displayPos = _deviceConfig.getLayoutPosition();

        if (displayPos == null) {
            _b3D4ClientError.setErrorCode(CONFIG_LAYOUT_MISSING.getValue());
            _registrantListener.onError(_b3D4ClientError, _captureSettings);
            return;
        }

        InitNativeProcessorJNI(b3D4NativeProcessorListener, displayPos, processBufferV.mmBufferV.size(),_captureSettings.getDoManualReCalibration());

        updateDepthMapSetting();
        if(!_captureSettings.getRequest().equals(STRING_CANCEL_PROCESS)) {
            updateStitcherParameterJNI(_captureSettings.getFaceRectA(), _captureSettings.getFaceDistanceA(), _captureSettings.getxFormA());
            StartProcessJNI();
        }

        processBufferV.setisProcessing(true);
        for(int index = 0 ; index < processBufferV.mmBufferV.size() ;  index++) {
            String fileName = processBufferV.mmBufferV.get(index).first;
            byte[] bufferToSend = DiskService.mmapGet(fileName);
            if(bufferToSend != null && !_captureSettings.getRequest().equals(STRING_CANCEL_PROCESS) && !_isCancelProcessing) {
                addFrameToNativeJNI(bufferToSend.clone(),
                        processBufferV.mmBufferV.get(index).second.first, processBufferV.mmBufferV.get(index).second.second, mResolution, false);
            } else {
                LogService.w(TAG,"can't find file " + fileName + " or current request is  " + _captureSettings.getRequest().equals(STRING_CANCEL_PROCESS));
                if(_isCancelProcessing) {
                    // actually... if break because _isCancelProcessing, will set is processing to false out side of loop
                    processBufferV.setisProcessing(false);
                    break;
                }
            }
        }
        processBufferV.setisProcessing(false);
    }

    public void startProcessingFaceDetection() {

        if(_captureSettings.isDiagnostic())
            InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_DIAGNOSTIC.getValue());
        else
            InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_FACE.getValue());
        InitNativeProcessorJNI(b3D4NativeProcessorListener, "", 0,false);
        updateDepthMapSetting();
        StartFaceDetectionProcessJNI();

    }

    public void startGenerateFaceLandMark() {
        InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_FACELANDMARK.getValue());
        InitNativeProcessorJNI(b3D4NativeProcessorListener, "", _captureSettings.getDepthFrames(),false);
        updateDepthMapSetting();
        StartFaceLandMarkJNI();
    }

    public void startStreamDepth() {

        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                preparefolder(_captureSettings.getStreamDepthBufferID(), _captureSettings.isDebug());
            }
        });

        _calibDispErr = -1.0f;
        _calibErrCode = 0;
        _isCancelProcessing = false;

        InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_DEPTHCOMPUTE.getValue());
        LocalConfigService.updateSaveResultFlagJNI(_captureSettings.isDebug());
        InitNativeProcessorJNI(b3D4NativeProcessorListener, "", _captureSettings.getStreamDepthFrames(),_captureSettings.getDoManualReCalibration());
        updateDepthMapSetting();
        StartStreamDepthJNI();
    }



    public void startProcessingRecalibration() {

        _calibDispErr = -1.0f;
        _calibErrCode = 0;
        _isCancelProcessing = false;

        InitProcessingParemeterJNI(PROCESS_TYPE.PROCESS_TYPE_RECALIBRATION.getValue());
        LocalConfigService.updateSaveResultFlagJNI(_captureSettings.isDebug());
        InitNativeProcessorJNI(b3D4NativeProcessorListener, "", 1,false);
        updateDepthMapSetting();
        StartRecalibrationJNI();
    }

    public void cancelProcessing() {
        _isCancelProcessing = true;
        CancelProcessJNI();
    }

    private void preparefolder(String requesID, boolean isDebug) {
        String sessionFolder = GlobalResourceService.SESSIONS_FOLDER_PATH + "/" + requesID + "/";
        DiskService.createDirectory(new File(sessionFolder));
        if(isDebug) {
            DiskService.createDirectory(new File(sessionFolder + "Results/L"));
            DiskService.createDirectory(new File(sessionFolder + "Results/M"));
            DiskService.createDirectory(new File(sessionFolder + "Results/R"));
            DiskService.createDirectory(new File(sessionFolder + "Results/D"));
        }
    }

    /* update narive depth lib settings */
    private void updateDepthMapSetting() {
        _depthMapSettings = new DepthMapSettings();
        _depthMapSettings.depthCameraType = DepthMapSettings.DepthCameraType.DEPTHCAM_INU25.ordinal();   // depthCameraType will get from calib.data, so it won't work
        _depthMapSettings.sceneMode = DepthMapSettings.SCENE_MODE.NORMAL.ordinal();
        _depthMapSettings.optimizeMode = DepthMapSettings.OPTIMIZE_MODE.OPTIMIZE_QUALITY.ordinal();
        _depthMapSettings.depthRegistration = DepthMapSettings.DEPTH_REGISTRATION.REGISTER_L.ordinal();
        _depthMapSettings.depthScale = 1.00f;
        _depthMapSettings.depthUnit = 0.02f;
        _depthMapSettings.ROI_X = 0;
        _depthMapSettings.ROI_Y = 0;
        _depthMapSettings.ROI_Width = 800;
        _depthMapSettings.ROI_Height = 1280;
        _depthMapSettings.useDSP = false;
        _depthMapSettings.useDepthRange = true;
        _depthMapSettings.useIRThres = false;
        _depthMapSettings.useTwoDispMaps = true;
        setDepthMapSettingsJNI(_depthMapSettings);
    }

    public void updateNativeDebugFlag(String bufferID, final boolean debug) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            public void run() {
                preparefolder(_captureSettings.getStreamDepthBufferID(), debug);
            }
        });
        LocalConfigService.updateSaveResultFlagJNI(debug);
    }

    protected B3D4NativeProcessorListener b3D4NativeProcessorListener = new B3D4NativeProcessorListener() {
        @Override
        public void onError(int errorcode) {
            /* get a native error stopNativeProcess */
            LogService.i(TAG,"errorcode : " + errorcode);
            _registrantListener.onError(new B3D4ClientError(errorcode),_captureSettings);
            return;
        }

        @Override
        public void onFrameEnough() {
            /* we collected enough frames stop streaming */
            LogService.v(TAG,"onFrameEnough call");
        }

        @RequiresApi(api = Build.VERSION_CODES.N)
        @Override
        public void onProcessDone(byte[] stitchData) throws IOException {

            /* we finished stitcher, stop native cameraStreamProcessor  and clear the data*/
            StopProcessJNI();

            String request = _captureSettings.getRequest();
            String requestID = _captureSettings.getRequestID();
            String bufferID = _captureSettings.getMergeBufferID();
            String camPos = _deviceConfig.getLayoutPosition();

            boolean isDebug = false; // host can't receive such a big zip, hard code false first
            String deviceID = GlobalResourceService.getDeviceId();

            String sessionPath = GlobalResourceService.SESSIONS_FOLDER_PATH + "/" + bufferID + "/";

            String debugFile = "debug_" + camPos + "_" + deviceID + "_" + bufferID + ".zip";
            String debugPath = sessionPath + debugFile;
            if (isDebug) {
                String tempPath = GlobalResourceService.ARC_TEMP_PATH + debugFile;
                if (DiskService.fileExists(debugPath)) DiskService.deleteFile(debugPath);
                DiskService.zipToPath(sessionPath, tempPath, "B3D4"+deviceID.substring(deviceID.length()-4)+bufferID.substring(bufferID.length()-4));

                if (!DiskService.renameFolder(tempPath, debugPath)) {
                    LogService.e(TAG, "Unable to move zip file");
                    DiskService.deleteFile(tempPath);
                    isDebug = false;
                };
            }

            // Check that required files exist
            int index = 0;
            File[] zipfiles = new File[9];
            String[] paths = new String[9];
            paths[index] = sessionPath;
            zipfiles[index++] = new File("Color_1.jpg");  //color image
            paths[index] = sessionPath;
            zipfiles[index++] = new File("Depth_1.png"); //stitcher image
            paths[index] = sessionPath;
            zipfiles[index++] = new File("Confidence_1.png"); // confidence map
            paths[index] = sessionPath;
            zipfiles[index++] = new File("RMSE_1.png"); // RMSE
            paths[index] = sessionPath;
            zipfiles[index++] = new File("Mask_1.png"); // MASK
            paths[index] = sessionPath;
            zipfiles[index++] = new File("processing.log");
            paths[index] = ARC_CLIENT_PATH;
            zipfiles[index++] = new File("CalibrationFiles/depthCam.yml"); //depthCam yml
            paths[index] = ARC_CLIENT_PATH;
            zipfiles[index++] = new File("CalibrationFiles/midCam.yml"); //midCam yml
            if (isDebug) {
                paths[index] = sessionPath;
                zipfiles[index++] = new File(debugFile);
            }

            String missing = "";
            for (int i=0; i<index; i++) {
                String file = zipfiles[i].toString();
                String filePath = paths[i]+file;
                LogService.d(TAG, "Zip checking file : "+filePath);
                if (!DiskService.fileExists(filePath)) {
                    LogService.w(TAG, "Zip missing : "+filePath);
                    // !!! Should create list, then join list
                    if (!missing.isEmpty()) missing += ",";
                    missing += file;
                }
            }

            if (!missing.isEmpty()) {
                _b3D4ClientError.setErrorCode(FILE_MISSING.getValue());
                _registrantListener.onError(_b3D4ClientError, _captureSettings);
                LogService.e(TAG, "Zip missing files");
                return;
            }

            byte[] bytes = null;
            bytes = DiskService.zipFiles(paths, zipfiles);
            if (isDebug) {
                DiskService.deleteFile(debugPath);
            }
            LogService.v(TAG, "zip end");

            if (bytes == null || bytes.length == 0) {
                LogService.e(TAG, "stitch request data is missing");
                _b3D4ClientError.setErrorCode(FILE_MISSING.getValue());
                _b3D4ClientError.setErrorMessage(FILE_MISSING.getValue(),
                        "ERROR_EMPTY_MERGE_FILES");
                _registrantListener.onError(_b3D4ClientError, _captureSettings);
                return;
            }

            String filename = camPos + ".zip";
            BufferIdResponseTo bufferIdResponseTo = new BufferIdResponseTo();
            bufferIdResponseTo.setStatus(NetWorkMessage.Status.OK.getName());
            bufferIdResponseTo.setResponseTo(request);
            bufferIdResponseTo.setRequestId(requestID);
            bufferIdResponseTo.setBufferId(bufferID);
            String bufferIdResponseToMsg = gson.toJson(bufferIdResponseTo);
            LogService.i(TAG, bufferIdResponseToMsg);
            _networkService.sendMessage(bufferIdResponseToMsg);

            String ts = TimeStampService.getTimestamp(null, GlobalResourceService.TIME_DIFFERENCE);
            MergeRequest mergeRequest = new MergeRequest();
            mergeRequest.setRequest(NetWorkMessage.RequestType.MERGE.getName());
            mergeRequest.setRequestId(requestID);
            mergeRequest.setBufferId(bufferID);
            mergeRequest.setFilename(filename);
            mergeRequest.setFileSize(String.valueOf(bytes.length));
            mergeRequest.setTs(ts);
            String reCalType = _captureSettings.getDoManualReCalibration() ? RECALIBRATION_MANUAL : RECALIBRATION_AUTO;
            mergeRequest.setRecalibType(reCalType);
            mergeRequest.setCaldispError(String.valueOf(_calibDispErr));
            mergeRequest.setCalibResult((_calibErrCode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_ERROR.getValue()
                    || _calibErrCode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_CHANGE.getValue()) ? STRING_TRUE : STRING_FALSE);
            String mergeRequestMsg = gson.toJson(mergeRequest);
            LogService.i(TAG, mergeRequestMsg);
            _networkService.sendBinaryToHost(mergeRequestMsg, bytes,true);

            // for now only callback to MainActivity to trigger check buffer to release
            // so pass null to MainActivity
            if(_registrantListener != null) _registrantListener.onProcessDone(null);
        }

        @Override
        public void onFaceDetectDone(float[] headPoseInfo) {
            if (_captureSettings.isTrackingFace()) {
                _captureSettings.setTrackingFace(headPoseInfo);
            }
        }

        public void onGenFaceLandMarkDone() {
            byte[] landmark = DiskService.readFileToByte(ARC1_FACE_LANDMARk_FOLDER_PATH + "/facelandmark.yml");

            StreamCaptureRequest streamCaptureRequest = new StreamCaptureRequest();
            streamCaptureRequest.setRequest(_captureSettings.getRequest());
            streamCaptureRequest.setRequestId(_captureSettings.getRequestID());
            streamCaptureRequest.setBufferId(_captureSettings.getStreamCaptureBufferID());
            streamCaptureRequest.setFrameType(FRAME_TYPE_FACELM);
            streamCaptureRequest.setFilesize(String.valueOf(landmark.length));

            String streamCaptureRequestStr = gson.toJson(streamCaptureRequest);
            LogService.i(TAG,"send faceLandmark to host " + streamCaptureRequestStr);
            _networkService.sendBinaryToHost(streamCaptureRequestStr, landmark, true);

            _registrantListener.onGenFaceLandMarkDone();
        }

        public void onRecalibrationDone(int errorcode, float calibDispErr) {
            LogService.d(TAG," errorcode : " + errorcode + ", calibDispErr : " + calibDispErr);

            _calibDispErr = calibDispErr;
            _calibErrCode = errorcode;

            /* only send reponse to host when request is recalibration */
            if(_captureSettings.getRequest().toLowerCase().equals(STRING_RECALIBRATION)) {
                boolean success = (errorcode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_CHANGE.getValue() ||
                        errorcode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_ERROR.getValue()||
                        errorcode == B3D4ClientError.B3D_ERROR_CATEGORY.B3D_OK.getValue());

                String status = success ? NetWorkMessage.Status.OK.getName() : NetWorkMessage.Status.ERROR.getName();
                ResponseTo responseTo = new ResponseTo();
                responseTo.setStatus(status);
                responseTo.setResponseTo(_captureSettings.getRequest());
                responseTo.setRequestId(_captureSettings.getRequestID());
                String recalibrationResponseToMsg = gson.toJson(responseTo);
                LogService.i(TAG, "send recalibration message to host : " + recalibrationResponseToMsg);
                _networkService.sendMessage(recalibrationResponseToMsg);
                AppStatusManager.getInstance().setState(CAMERA_PROCESSING_COMPLETED);
            }
            if(_registrantListener != null)
                _registrantListener.onRecalibrationDone(errorcode, calibDispErr);
        }

        public void onStreamDepthDone(byte[] bData, long timeStampL) {

            String currRequest = _captureSettings.getRequest().toLowerCase();
            boolean isDone = false;
            String fileNameL  = null;
            String frameIdx = null;

            switch (currRequest) {
                case STRING_BUFFER_CAPTURE :
                    isDone = false;
                    fileNameL = String.format("%03d",1) +
                            "_"+String.format("%06d",timeStampL / NSTOMS) + "_" + String.format("%05d",1);
                    frameIdx = "1";
                    break;
                case STRING_STREAM_DEPTH : {
                    _depthFrameStatistics.StreamDepthDepthIndex++;
                    isDone = (_depthFrameStatistics.StreamDepthDepthIndex ==
                            (_captureSettings.getStreamDepthFrames() + _captureSettings.getStreamDepthStartIndex() -1 ));
                    fileNameL = String.format("%03d",_depthFrameStatistics.StreamDepthDepthIndex) +
                            "_"+String.format("%06d",timeStampL / NSTOMS) + "_" + String.format("%05d",_depthFrameStatistics.StreamDepthDepthIndex);
                    frameIdx = String.valueOf(_depthFrameStatistics.StreamDepthDepthIndex);
                    break;
                }
            }


            StreamCaptureRequest streamCaptureRequest = new StreamCaptureRequest();
            streamCaptureRequest.setRequest(_captureSettings.getRequest());
            streamCaptureRequest.setRequestId(_captureSettings.getRequestID());
            streamCaptureRequest.setBufferId(_captureSettings.getStreamDepthBufferID());
            streamCaptureRequest.setFrameType(FRAME_TYPE_DEPTH);
            streamCaptureRequest.setFilesize(String.valueOf(bData.length));
            streamCaptureRequest.setFilenameDepth(fileNameL);
            streamCaptureRequest.setFrameIndex(frameIdx);
            streamCaptureRequest.setBufferFinish((isDone ? STRING_TRUE : STRING_FALSE));

            if (!AppStatusManager.getInstance().getState().equals(CAMERA_RECORDING)) {
                AppStatusManager.getInstance().setState(CAMERA_PROCESSING_OR_TRANSFER);
            }

            if(isDone) {
                AppStatusManager.getInstance().setState(CAMERA_RECEIVED_COMMAND);
                _registrantListener.onStreamDepthDone(null,0);
                String reCalType = _captureSettings.getDoManualReCalibration() ? RECALIBRATION_MANUAL : RECALIBRATION_AUTO;
                streamCaptureRequest.setRecalibType(reCalType);
                streamCaptureRequest.setCaldispError(String.valueOf(_calibDispErr));
                streamCaptureRequest.setCalibResult((_calibErrCode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_ERROR.getValue()
                        || _calibErrCode == B3D4ClientError.B3D_RECALIBRATION_ERROR.CALIB_NO_CHANGE.getValue()) ? STRING_TRUE : STRING_FALSE);
            }

            String streamCaptureRequestStr = gson.toJson(streamCaptureRequest);
            _networkService.sendBinaryToHost(streamCaptureRequestStr, bData, true);

            // StreamDepthNotification is by Max's request
            // make sure send notification after sending the last binary,
            if(isDone) {
                StreamDepthNotification streamDepthNotification = new StreamDepthNotification();
                streamDepthNotification.setNotification(_captureSettings.getRequest());
                streamDepthNotification.setCaptureCompleted(true);
                streamDepthNotification.setBufferId(_captureSettings.getStreamDepthBufferID());
                String streamDepthNotificationStr = gson.toJson(streamDepthNotification);
                _networkService.sendMessage(streamDepthNotificationStr);
            }
        }

        public void onProcessFinished(String who) {
            LogService.i(TAG,"finishing : " + who);
            if(_registrantListener != null) _registrantListener.onProcessFinished(who);
        }
    };

    private native void setDepthMapSettingsJNI(DepthMapSettings settings);

    /* native side reset compute container, control flag, uses process type to decide which start add frame flag
         * should enable, startAddProcessingFrames or startAddFaceDetectFrames
        */
    public native void InitProcessingParemeterJNI(int processType);

    /* init stitcher native class,  add native listener, root folder and session folder to CameraStreamProcessorImpl */
    public native void InitNativeProcessorJNI(B3D4NativeProcessorListener b3d4nativeListener,
                                              String displayPos, int processFrame, boolean needManualRecalib);

    /* update faceRect, faceDistance, camera xForm to stitcher  */
    public native void updateStitcherParameterJNI(double[] faceRect, double[] faceDistance, double[] xForm);

    /* add frame to Native */
    public native void addFrameToNativeJNI(byte[] byteData, long frameCount, long timestamp, int mResolution, boolean isCapture);

    /* native side start do depth compute */
    public native void StartProcessJNI();

    /* native side start do face detection, init doFaceDetectWork thread */
    public native void StartFaceDetectionProcessJNI();

    /* native side start do depth computation, init FaceLandMark thread */
    public native void StartFaceLandMarkJNI();

    /* native side start do recalibration, init doDepthWork thread */
    public native void StartRecalibrationJNI();

    /* native side start do depth computation, init doDepthWork thread */
    public native void StartStreamDepthJNI();

    /* native side start do single depth computation, init doSingleDepthWork thread */
    public native byte[] StartSingleDepthComputation(byte[] byteData, int width, int height, float imageScale);

    /* native side stop collect frames and stop depth compute */
    public native void StopProcessJNI();

    /* native side stop collect frames and stop depth compute */
    public native void CancelProcessJNI();

    /* set session folder name to native side */
    public static native void setProcessingSessionFolder(String sessionFolder);
}
