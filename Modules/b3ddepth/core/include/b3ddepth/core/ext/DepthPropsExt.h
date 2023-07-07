#pragma once

namespace b3dd {

    // This enum maps to DepthCameraSettings::DepthCameraType
    enum DepthCameraType {
        DEPTHCAM_SERES25 = 0,
        DEPTHCAM_B3D3    = 1,
        DEPTHCAM_02A25   = 2,
        DEPTHCAM_INU25   = 3,
        DEPTHCAM_B3D4    = 4,
        NUM_DEPTHCAMS    = 5
    };

    struct DepthProps {
        int minScanDistance;  // nearest matching distance (mm)
        int maxScanDistance;  // furtherest matching distance (mm)
        int stereoBlockSize;  // block match window size (pixels)
        int maxClipDistance;  // clip depth map at some distance (mm)
                              // currently separating max matching dist & max clip distance
    };

    static const DepthProps DEPTH_PROPS[NUM_DEPTHCAMS] = {
        { 150,  3000, 13, 1500},  // SERES25
        { 150,  2000, 17, 1500},  // B3D3
        { 200,  3000, 15, 1200},  // 02A25
        { 250,   700, 15, 1200},  // INU25
        { 200,   650, 23,  650}   // B3D4
    };


    inline std::string getDepthCamTypeString(DepthCameraType depthCamType) {
        switch (depthCamType) {
        case DEPTHCAM_SERES25: return "SERES25";
        case DEPTHCAM_B3D3   : return "B3D3";
        case DEPTHCAM_02A25  : return "02A25";
        case DEPTHCAM_INU25  : return "INU25";
        case DEPTHCAM_B3D4   : return "B3D4";
        default:
            //logError("getDepthCamTypeString() error - unknown depth cam type!");
            break;
        }

        return "N/A";
    }

    inline DepthCameraType getDepthCamTypeFromIndex(int depthCameraTypeIndex) {
        switch (depthCameraTypeIndex) {
        case 0: return DEPTHCAM_SERES25;
        case 1: return DEPTHCAM_B3D3;
        case 2: return DEPTHCAM_02A25;
        case 3: return DEPTHCAM_INU25;
        case 4: return DEPTHCAM_B3D4;
        default:
            //logError("getDepthCamTypeFromIndex() error - unknown depth cam type!");
            return NUM_DEPTHCAMS;
        }
    }

}  // End of namespace b3dd
