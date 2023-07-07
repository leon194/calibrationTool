package com.bellus3d.android.arc.b3d4client.camera;

/**
 * DepthCamera frames
 */

public class B3DImage {

    public enum B3DImageType {
        B3DIMAGE_UNCHANGED,     // Unknown type
        B3DIMAGE_RGB,           // 24-bit rgb
        B3DIMAGE_MONO,          // 8-bit gray scale (IR image)
        B3DIMAGE_DEPTH_16,      // 16-bit depth
        B3DIMAGE_DEPTH_32,      // 32-bit floating point depth
    }


    B3DImage() {
        imageType = B3DImageType.B3DIMAGE_DEPTH_16;
    }


    B3DImageType imageType;

    byte[] imageData;

}
