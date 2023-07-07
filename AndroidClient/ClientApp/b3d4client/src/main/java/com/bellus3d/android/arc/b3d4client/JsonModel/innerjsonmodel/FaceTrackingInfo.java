package com.bellus3d.android.arc.b3d4client.JsonModel.innerjsonmodel;

public class FaceTrackingInfo {

    /**
     * distance : {"x":"0.0","y":"0.0","z":"0.0"}
     * rotation : {"x":"0.0","y":"0.0","z":"0.0"}
     */

    private Position distance;
    private Position rotation;
    private Position facerect;

    public Position getDistance() {
        return distance;
    }

    public void setDistance(Position distance) {
        this.distance = distance;
    }

    public Position getRotation() {
        return rotation;
    }

    public void setRotation(Position rotation) { this.rotation = rotation; }

    public Position getFaceRect() {
        return facerect;
    }

    public void setFaceRect(Position facerect) { this.facerect = facerect; }
}
