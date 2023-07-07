#pragma once

#include <memory>

namespace b3dd {

    //enum OPTIMIZE_MODE {
    //    OPTIMIZE_QUALITY  = 0,
    //    OPTIMIZE_STANDARD = 1,  // Beta, don't use yet
    //    OPTIMIZE_SPEED    = 2
    //};

    enum OPTIMIZE_MODE {
        OPTIMIZE_FULL_RES = 0,
        OPTIMIZE_HALF_RES = 1,  // Beta, don't use yet
        OPTIMIZE_QUARTER_RES = 2,
        OPTIMIZE_QQUARTER_RES = 3
    };

    enum DEPTH_REGISTRATION {
        REGISTER_L,
        REGISTER_R,
        REGISTER_RGB
    };

    enum SCENE_MODE {
        NORMAL    = 0,
        FLATWALL  = 1,
        OUTDOOR   = 2,
        INDOOR    = 3,
        FACEID    = 4,
        DEMO      = 5,
        CALIB     = 6,
        INTERNAL  = 7
    };

    enum DEPTH_RESOLUTION {
        P720,
        VGA,
        HVGA,
        QVGA
    };

    struct ROI {
        ROI(int _x = -1, int _y = -1, int _width = -1, int _height = -1) {
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

    struct DepthConfig {
        DepthConfig() {
            sceneMode         = NORMAL;
            optimizeMode      = OPTIMIZE_QUARTER_RES;
            depthRegistration = REGISTER_L;
            depthScale        = 1.00f;
            depthUnit         = 0.02f;

            ROI_L = ROI();
        }

        SCENE_MODE sceneMode;                  // sets scene for different applications
        OPTIMIZE_MODE optimizeMode;            // sets depth processing mode
        DEPTH_REGISTRATION depthRegistration;  // sets which camera depth map register to
        float depthScale;                      // sets depth output resolution
        float depthUnit;                       // sets depth accuracy and range
        ROI ROI_L;                             // sets the region of interests of depth computation
    };

    using DepthConfigPtr = std::shared_ptr<DepthConfig>;

}  // End of namespace b3dd
