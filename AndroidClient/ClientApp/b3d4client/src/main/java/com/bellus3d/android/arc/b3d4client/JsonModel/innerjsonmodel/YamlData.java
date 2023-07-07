package com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel;

import java.util.List;

public class YamlData {

    private List<String> cameraMatrixData;
    private List<String> RData;
    private List<String> TData;
    private List<String> imageSize;
//    private List<String> distCoeffs;

    public List<String> getCameraMatrixData() {
        return cameraMatrixData;
    }

    public void setCameraMatrixData(List<String> cameraMatrixData) {
        this.cameraMatrixData = cameraMatrixData;
    }

    public List<String> getRData() {
        return RData;
    }

    public void setRData(List<String> RData) {
        this.RData = RData;
    }

    public List<String> getTData() {
        return TData;
    }

    public void setTData(List<String> TData) {
        this.TData = TData;
    }

    public List<String> getImageSize() {
        return imageSize;
    }

    public void setImageSize(List<String> imageSize) {
        this.imageSize = imageSize;
    }

//    public List<String> getDistCoeffs() {
//        return distCoeffs;
//    }
//
//    public void setDistCoeffs(List<String> distCoeffs) {
//        this.distCoeffs = distCoeffs;
//    }
}
