package com.bellus3d.android.arc.b3d4client.camera;

/**
 * Settings that configure the depth map quality
 */

public class DepthMapSettings {

    private static final String TAG = "B3D4NewClient-DepthMapSettings";

    /**
     * Depth map registration space
     */
    public enum DEPTH_REGISTRATION {
        REGISTER_L,
        REGISTER_R,
        REGISTER_RGB
    }

    /**
     * Depth map processing optimization mode
     */
    public enum OPTIMIZE_MODE {
        OPTIMIZE_QUALITY,
        OPTIMIZE_STANDARD,
        OPTIMIZE_SPEED
    }

    /**
     * Depth map processing scene mode
     */
    public enum SCENE_MODE {
        NORMAL,
        FLATWALL,
        OUTDOOR,
        INDOOR,
        FACEID,
        DEMO
    };

    enum DEPTH_RESOLUTION {
        P720,
        VGA,
        HVGA,
        QVGA
    };

    class ROI {
        public ROI() {
            x = y = width = height = -1;
        }
        public ROI(int _x, int _y, int _width, int _height) {
            x = _x;
            y = _y;
            width = _width;
            height = _height;
        }

        int x;
        int y;
        int width;
        int height;
    };

    public enum DepthCameraType {
        DEPTHCAM_SERES25,
        DEPTHCAM_B3D3,
        DEPTHCAM_02A25,
        DEPTHCAM_INU25,
        NUM_DEPTHCAMS
    };
    /**
     * Copy constructor of DepthMapSettings
     * @param settings the settings to copy from
     */
    DepthMapSettings(DepthMapSettings settings) {

        // public configs
        this.depthCameraType   = settings.depthCameraType;
        this.sceneMode         = settings.sceneMode;
        this.optimizeMode      = settings.optimizeMode;
        this.depthRegistration = settings.depthRegistration;
        this.depthScale        = settings.depthScale;
        this.depthUnit         = settings.depthUnit;
        this.ROI_X             = settings.ROI_X;
        this.ROI_Y             = settings.ROI_Y;
        this.ROI_Width         = settings.ROI_Width;
        this.ROI_Height        = settings.ROI_Height;

        //Ext configs
        this.useDSP            = settings.useDSP;
        this.useIRThres        = settings.useIRThres;
        this.useTwoDispMaps    = settings.useTwoDispMaps;
        this.useDepthRange     = settings.useDepthRange;

        this.useDispRange      = settings.useDispRange;
        this.useNoiseRemoval   = settings.useNoiseRemoval;
    }


    /**
     * Constructor
     */
    public DepthMapSettings() {

        // public configs
        this.depthCameraType   = DepthCameraType.DEPTHCAM_SERES25.ordinal();
        this.sceneMode         = SCENE_MODE.NORMAL.ordinal();
        this.optimizeMode      = OPTIMIZE_MODE.OPTIMIZE_QUALITY.ordinal();
        this.depthRegistration = DEPTH_REGISTRATION.REGISTER_L.ordinal();
        this.depthScale        = 1.00f;
        this.depthUnit         = 0.02f;
        this.ROI_X             = -1;
        this.ROI_Y             = -1;
        this.ROI_Width         = -1;
        this.ROI_Height        = -1;

        //Ext configs
        this.useDSP            = false;
        this.useIRThres        = false;
        this.useTwoDispMaps    = true;
        this.useDepthRange     = true;

        this.useDispRange      = false;
        this.useNoiseRemoval   = false;

    }

    // public configs
    public int depthCameraType;       // sets depth camera type
    public int sceneMode;                  // sets scene for different applications
    public int optimizeMode;            // sets depth processing mode
    public int depthRegistration;  // sets which camera depth map register to
    public float depthScale;                      // sets depth output resolution
    public float depthUnit;                       // sets depth accuracy and range
    public int ROI_X,ROI_Y,ROI_Width,ROI_Height;                             // sets the region of interests of depth computation


    //Ext configs, for debug only
    public boolean useDSP;

    // Processing Properties
    public boolean useIRThres;       // whether to use IR to threshold the image or not
    public boolean useTwoDispMaps;   // use second disp map to help improve quality
    public boolean useDepthRange;    // first estimate foreground depth range

    // Related to DepthCamera property
    int minScanDistance;  // nearest matching distance (mm)
    int maxScanDistance;  // furtherest matching distance (mm)
    int stereoBlockSize;  // block match window size (pixels)
    int maxClipDistance;  // clip depth map at some distance (mm)

    // Becoming obsolete
    public boolean useNoiseRemoval;  // for flat wall tests
    public boolean useDispRange;     // use disparity range to improve output disparity map

}
