#include <jni.h>
#include <string>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <b3d4client/B3DCamera.h>
#include <b3d4client/B3DCameraError.h>
#include <b3d4client/B3DCameraStreamListener.h>
#include <b3d4client/CameraStreamProcessor.h>
#include <b3d4client/DepthStitcher.h>
#include <thread>

#include "jnihelpers.h"

#define CALIBRATION_FOLDER_PATH "/sdcard/Bellus3d/Arc/calibrationtool/"

/* give it a initial value, but will update from Java side */
static std::string ARC_ROOT_FOLDER_PATH = "/sdcard/Bellus3d/Arc/ArcClient/";
static std::string ARC_PROCESSING_SESSION_FOLDER = "/sdcard/Bellus3d/Arc/ArcClient/sessions/default/";
static std::string ARC1_LANDMARK_FOLDER_PATH = "/sdcard/Bellus3d/Arc/ArcClient/Arc1/FaceLandMark/";

#define AE_EXP_GAIN_PARAM_FILE_NAME "/mnt/vendor/ae.file"
#define AWB_GAIN_PARAM_FILE_NAME "/mnt/vendor/awb.file"

typedef unsigned short cmr_u16;
typedef unsigned int cmr_u32;

typedef signed short cmr_s16;
typedef signed int cmr_s32;

struct ae_exposure_param {
    cmr_u32 cur_index;
    cmr_u32 line_time;
    cmr_u32 exp_line;
    cmr_u32 exp_time;
    cmr_s32 dummy;
    cmr_s32 frm_len;
    cmr_s32 frm_len_def;
    cmr_u32 gain;			/*gain = sensor_gain * isp_gain */
    cmr_u32 sensor_gain;
    cmr_u32 isp_gain;
    cmr_s32 target_offset;
    cmr_s32 bv;
    cmr_u32 table_idx;
};

struct ae_exposure_param_switch {
    cmr_u32 target_offset;
    cmr_u32 exp_line;
    cmr_u32 exp_time;
    cmr_s32 dummy;
    cmr_u32 frm_len;
    cmr_u32 frm_len_def;
    cmr_u32 gain;
    cmr_u32 table_idx;
};

struct awb_save_gain {
    cmr_u16 r;
    cmr_u16 g;
    cmr_u16 b;
    cmr_u16 ct;
};

extern "C" {
static jclass g_jclass = nullptr;
static jobject g_obj = nullptr;
static jobject g_surfaceJavaL = nullptr;
static jobject g_surfaceJavaR = nullptr;
static jobject g_surfaceJavaM = nullptr;
}

using namespace std;
using namespace cv;
using namespace b3d4;

class CameraStreamProcessListener;

class StitcherProcessListener;

static CameraStreamProcessorPtr cameraStreamProcessorPtr;

static std::shared_ptr<CameraStreamProcessListener> cameraStreamProcessListener;

static std::shared_ptr<StitcherProcessListener> stitcherProcessListener;

static B3DCameraPtr b3dCameraPtr;

static B3DStitcherPtr b3dStitcherPtr;

class PreviewStreamListener;

static std::shared_ptr<PreviewStreamListener> streamListenerPtr;

static DepthConfigExtPtr depthConfigPtr;

static bool isSaveResultFrame;


