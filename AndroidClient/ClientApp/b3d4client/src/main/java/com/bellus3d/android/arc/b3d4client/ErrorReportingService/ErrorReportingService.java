package com.bellus3d.android.arc.b3d4client.ErrorReportingService;

import com.bellus3d.android.arc.b3d4client.AppStatusManager;
import com.bellus3d.android.arc.b3d4client.JsonModel.ErrorResponseTo;
import com.bellus3d.android.arc.b3d4client.LogService.LogService;
import com.bellus3d.android.arc.b3d4client.MessageService.NetWorkMessage;
import com.bellus3d.android.arc.b3d4client.NetworkService.NetworkService;
import com.bellus3d.android.arc.b3d4client.camera.B3DCaptureSettings;
import com.google.gson.Gson;

import static com.bellus3d.android.arc.b3d4client.GlobalResourceService.TAG;

public class ErrorReportingService {
    private NetworkService _networkService;

    public void setNetworkService(NetworkService networkService) {
        _networkService = networkService;
    }

    public void sendWarningToHost(B3D4ClientError error, B3DCaptureSettings captureSetting) {
        String ErrorCategories = error.getErrorCategories(error.getErrorCode());
        String ErrorMessage = error.getErrorMessage(error.getErrorCode());
        LogService.d(TAG,"errorcode : " + error.getErrorCode() + ", errormessage : " + ErrorMessage);

        Gson gson = new Gson();
        ErrorResponseTo errorResponseTo = new ErrorResponseTo();
        errorResponseTo.setStatus(ErrorCategories);
        errorResponseTo.setResponseTo(captureSetting.getRequest());
        errorResponseTo.setRequestId(captureSetting.getRequestID());
        errorResponseTo.setError(ErrorMessage);
        String errorResponseToMsg = gson.toJson(errorResponseTo);
        LogService.i(TAG, errorResponseToMsg);
        _networkService.sendMessage(errorResponseToMsg);
    }

    public void sendErrorToHost(B3D4ClientError error, B3DCaptureSettings captureSetting) {
        String ErrorCategories = error.getErrorCategories(error.getErrorCode());
        String ErrorMessage = error.getErrorMessage(error.getErrorCode());
        LogService.d(TAG,"errorcode : " + error.getErrorCode() + ", errormessage : " + ErrorMessage);

        Gson gson = new Gson();
        ErrorResponseTo errorResponseTo = new ErrorResponseTo();
        errorResponseTo.setStatus(ErrorCategories);
        errorResponseTo.setResponseTo(captureSetting.getRequest());
        errorResponseTo.setRequestId(captureSetting.getRequestID());
        errorResponseTo.setError(ErrorMessage);
        String errorResponseToMsg = gson.toJson(errorResponseTo);
        LogService.i(TAG, errorResponseToMsg);
        AppStatusManager.getInstance().setState(AppStatusManager.AppState.CAMERA_RECORDING_ERROR);
        _networkService.sendMessage(errorResponseToMsg);
    }

    public void sendErrorToHost(B3D4ClientError error, NetWorkMessage nMsg) {
        String ErrorCategories = error.getErrorCategories(error.getErrorCode());
        String ErrorMessage = error.getErrorMessage(error.getErrorCode());
        LogService.d(TAG,"errorcode : " + error.getErrorCode() + ", errormessage : " + ErrorMessage);

        Gson gson = new Gson();
        ErrorResponseTo errorResponseTo = new ErrorResponseTo();
        errorResponseTo.setStatus(ErrorCategories);
        errorResponseTo.setResponseTo(nMsg.request);
        errorResponseTo.setRequestId(nMsg.request_id);
        errorResponseTo.setError(ErrorMessage);
        String errorResponseToMsg = gson.toJson(errorResponseTo);
        LogService.i(TAG, errorResponseToMsg);
        AppStatusManager.getInstance().setState(AppStatusManager.AppState.CAMERA_RECORDING_ERROR);
        _networkService.sendMessage(errorResponseToMsg);
    }

}
