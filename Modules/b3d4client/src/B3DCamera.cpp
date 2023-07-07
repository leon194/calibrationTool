#include "B3DCamera.h"


#include "B3DCamera_B3D4.h"
#include "B3DCamera_Files.h"


using namespace std;

namespace b3d4 {

B3DCameraPtr B3DCamera::newB3DCamera(DCamType type) {

    vector<std::string> supportedDevices = B3DCamera::detectSupportedDevices();

    switch (type) {
        case B3D4 :
            return shared_ptr<B3DCamera>(new B3DCamera_B3D4(type));
        case FILES :
            return shared_ptr<B3DCamera>(new B3DCamera_Files(type));
        default :
            return shared_ptr<B3DCamera>(new B3DCamera_Files(type));
    }
}

std::vector<std::string> B3DCamera::detectSupportedDevices() {

    vector<std::string> supportedDevices;

#ifdef B3D3_AVAILABLE
    supportedDevices.push_back("B3D3");
    return supportedDevices;
#endif

#ifdef TARGET_OS_IPHONE
    supportedDevices.push_back("IPHX");
    return supportedDevices;
#endif

#ifdef X86_64
    supportedDevices.push_back("SPRD");
    TLog::log(TLog::LOG_ERROR, "DepthCamera_B3D3 is not supported on x86_64 platform");
    return supportedDevices;
#endif

    return supportedDevices;
}


void B3DCamera::setCamPosition(std::string pos) {
    _cameraPosition = pos;
}

std::string B3DCamera::getCamPosition() {
    return _cameraPosition;
}

std::string B3DCamera::getFolderPath() {
    return _dataFolder;
}

void B3DCamera::setIsCancel(bool isCancel) {
    this->_isCancel = isCancel;
}

bool B3DCamera::getIsCancel() { return _isCancel; };


}
