#pragma once

#include <string>

// B3D Depth Core
#include <b3ddepth/core/B3DDef.h>
#include <b3ddepth/core/DepthConfig.h>

// B3D Depth Core Ext
#include <b3ddepth/core/ext/DepthPropsExt.h>


namespace b3dd {
    
    // For depth SDK internal processing, different from 3D Point Cloud generation
    //const int MIN_FOREGROUND_Z =  150;
    //const int MAX_FOREGROUND_Z = 3000;  // set maximum so that background objects won't be forced to match closer

    // These configurations remain the same for each application
    // should not turn ON / OFF
    struct DepthConfigExt : public DepthConfig {

        DepthConfigExt() {
            initialize();
            updateDepthConfig(DEPTHCAM_SERES25);
        }

        DepthConfigExt(DepthConfigPtr depthConfigPtr) {
            initialize();
            updateDepthConfig(DEPTHCAM_SERES25);

            this->sceneMode =         depthConfigPtr->sceneMode;
            this->optimizeMode =      depthConfigPtr->optimizeMode;
            this->depthRegistration = depthConfigPtr->depthRegistration;
            this->depthScale =        depthConfigPtr->depthScale;
            this->depthUnit =         depthConfigPtr->depthUnit;
            this->ROI_L =             depthConfigPtr->ROI_L;
#if defined ANDROID && defined USE_DSP
            // for release dsp version to customer purpose
            this->useDSP = true;
#endif
        }


        void initialize() {
            useDSP = false;

            useIRThres          = false;
            useForegroundMask   = false;
            useTwoDispMaps      = true;
            reduceDispBias      = false;
            useDepthRange       = true;

            useDispRange        = false;
            useNoiseRemoval     = false;
        }

        // Update config based on operating mode?
        // void updateDepthConfig(Mode: wall/indoor/outdoor)

        void updateDepthConfig(DepthCameraType depthCamType) {

            depthCameraType = depthCamType;

            stereoBlockSize = DEPTH_PROPS[depthCameraType].stereoBlockSize;
            minScanDistance = DEPTH_PROPS[depthCameraType].minScanDistance;
            maxScanDistance = DEPTH_PROPS[depthCameraType].maxScanDistance;
            maxClipDistance = DEPTH_PROPS[depthCameraType].maxClipDistance;

            uniqueRatio = 3;
        }

        DepthCameraType depthCameraType;

        bool useDSP;

        // Processing Properties
        bool useIRThres;       // whether to use IR to threshold the image or not
        bool useForegroundMask;// whether to use IR mask as foreground mask
        bool useTwoDispMaps;   // use second disp map to help improve quality
        bool reduceDispBias;   // the sub pixel positioning of BM has bias which the output
                               // is more likely to be in 0.5 pixel range and less likely to 
                               // be in 0 pixel range. Throw away pixels in 0.5 range will 
                               // reduce the bias
        bool useDepthRange;    // first estimate foreground depth range

        // Related to DepthCamera property
        int minScanDistance;  // nearest matching distance (mm)
        int maxScanDistance;  // furtherest matching distance (mm)
        int stereoBlockSize;  // block match window size (pixels)
        int maxClipDistance;  // clip depth map at some distance (mm)
        int uniqueRatio;      // unique ratio for block matching

        // Becoming obsolete
        bool useNoiseRemoval;  // for flat wall tests
        bool useDispRange;     // use disparity range to improve output disparity map
    };

    using DepthConfigExtPtr = std::shared_ptr<DepthConfigExt>;

    //void updateDepthConfig(DepthCameraType depthCameraType, DepthConfigExt& depthConfig);

    DLLEXPORT bool loadDepthConfigFromFile(std::string& filepath, DepthConfigExt& depthConfig);

    DLLEXPORT bool saveDepthConfigToFile(std::string& filepath, const DepthConfigExt& depthConfig);

}  // End of namespace b3dd
