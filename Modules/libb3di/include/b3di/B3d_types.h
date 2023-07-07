
#pragma once

namespace b3di {

enum MESH_RES {
    MESH_DEF = 0,
    MESH_LD = 1,
    MESH_SD = 2,
    MESH_HD = 3
};


enum ScanMode {
    SCAN_FRONTAL = 0,
    SCAN_THREE_QUARTER = 1,				// deprecated. behaves the same as SCAN_FRONTAL
    SCAN_FRONTAL_EXTENDED = 2,			// frontal face plus hair and neck
    SCAN_FULL_HEAD = 3,
};


enum TurningMode {
    AUTO_DETECT,							// automatically detect head rotation mode
    TURN_HEAD,							// Only the head is turning 
    TURN_CAMERA						// The camera is turning relative to the head
};

}