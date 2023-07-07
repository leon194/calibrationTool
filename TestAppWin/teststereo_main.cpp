#include <vector>
#include <iostream>
#include <string>
#include <algorithm>    // std::max
#include <fstream>
#include <set>

#include <numeric>

#include <process.h>

// Opencv
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

// b3di
#include "b3di/B3d_API.h"
#include "b3di/FileUtils.h"
#include "b3di/depthtoobj.h"

// B3D Core
#include "b3ddepth/core/B3DVersion.h"
#include "b3ddepth/core/DepthProcessor.h"

// B3D Utils
#include "b3ddepth/utils/B3DFileIO.h"
#include "b3ddepth/utils/imageutils.h"
#include "b3ddepth/utils/TProfile.h"
#include "b3ddepth/utils/TLog.h"

// B3D Ext
#include "b3ddepth/core//ext/DepthProcessorExt.h"
#include "b3ddepth/core//ext/DepthConfigExt.h"
#include "b3ddepth/core/ext/CameraCalibExt.h"
#include "b3ddepth/utils/ext/B3DCalibrationData.h"


using namespace std;
using namespace cv;

using namespace b3dd;



typedef std::pair<int, int> mypair;
//bool comparator(const mypair& l, const mypair& r) {
//    return l.first < r.first;
//}


void help() {
    cout
        << "--------------------------------------------------------------------------" << endl
        << "B3D DEPTH    Version: " << getB3DDEPTH_version() << endl
        << "B3D Internal Version: " << b3di::B3D_API::getVersionString() << endl
        << endl;
}


void loadB3DImages(const std::string filepath,
    std::vector<B3DImage>& imageContainer, std::vector<cv::String>& filenames) {

    std::string filepattern = "*.png";

    cout << "Loading images from: " << filepath << endl;

    imageContainer.clear();

    b3di::FileUtils::getDirFiles2(filepath + filepattern, filenames);

    for (auto filename : filenames) {

        B3DImage image;
        loadB3DImage(filepath + filename, image);

        if (image.isEmpty()) {
            cout << "Error: cannot load image: " << filename << endl;
            return;
        }
        else {
            imageContainer.push_back(image);
        }
    }
    return;
}

// Convert B3DImage to opencv Mat (make a copy)
void B3DImageToCvMat(const B3DImage& inImage, cv::Mat& outMat) {

    B3DImageType type = inImage.type();
    int matType;

    switch (type) {
    case B3DIMAGE_MONO: matType = CV_8UC1;
        break;
    case B3DIMAGE_DEPTH_16: matType = CV_16UC1;
        break;
    case B3DIMAGE_RGB: matType = CV_8UC3;
        break;
    default:
        //logError("Unsupported B3DImageType: " + b3dd::to_string(type) + " !");
        return;
    }

    outMat = Mat(inImage.rows(), inImage.cols(), matType);
    std::memcpy(outMat.data, inImage.data(), inImage.rowSize() * inImage.rows() * sizeof(unsigned char));
}

// Convert B3DImage to opencv Mat (make a copy)
void B3DImageToCvMat(const B3DImage& inImage, cv::Mat& outMat, B3DImageType type) {

    int matType;

    switch (type) {
    case B3DIMAGE_MONO: matType = CV_8UC1;
        break;
    case B3DIMAGE_DEPTH_16: matType = CV_16UC1;
        break;
    case B3DIMAGE_DISP_16: matType = CV_16SC1;
        break;
    case B3DIMAGE_RGB: matType = CV_8UC3;
        break;
    default:
        //logError("Unsupported B3DImageType: " + b3dd::to_string(type) + " !");
        return;
    }

    outMat = Mat(inImage.rows(), inImage.cols(), matType);
    std::memcpy(outMat.data, inImage.data(), inImage.rowSize() * inImage.rows() * sizeof(unsigned char));
}

// Convert opencv Mat to B3DImage(make a copy)
void cvMatToB3DImage(const cv::Mat& cvMat, B3DImage& b3dImage) {

    B3DImageType type;

    switch (cvMat.type()) {
    case CV_8UC1: type = B3DIMAGE_MONO;
        break;
    case CV_16UC1: type = B3DIMAGE_DEPTH_16;
        break;
    case CV_8UC3: type = B3DIMAGE_RGB;
        break;
    default:
        //logError("Unsupported B3DImageType: " + b3dd::to_string(type) + " !");
        return;
    }

    b3dImage = B3DImage(cvMat.rows, cvMat.cols, type);
    std::memcpy(b3dImage.data(), cvMat.data, cvMat.rows * cvMat.cols * cvMat.elemSize() * sizeof(unsigned char));

    return;
}

bool loadB3DImage(
    const std::string& filePath,
    B3DImage& b3dImage,
    const std::string& fileName,
    float imageScale
)
{
    cv::Mat mat = cv::imread(filePath + fileName, cv::IMREAD_GRAYSCALE);
    if (imageScale != 1.0f)
    {
        cv::resize(mat, mat, cv::Size(), imageScale, imageScale);
    }

    cvMatToB3DImage(mat, b3dImage);

    return !b3dImage.isEmpty();
}


void getFilePaths(
    const std::string& path,
    const std::string& namePattern,
    std::vector<cv::String>& filenames
)
{
    filenames.clear();
    b3di::FileUtils::getDirFiles2(path + namePattern, filenames);
    return;
}


void loadB3DImages(const std::string filepath,
    std::vector<B3DImage>& imageContainer, 
    std::vector<cv::String>& filenames,
    float imageScale
) {

    std::string filepattern = "*.png";

    getFilePaths(filepath, filepattern, filenames);

    for (auto filename : filenames) {

        B3DImage image;
        if (loadB3DImage(filepath, image, filename, imageScale))
        {
            imageContainer.push_back(image);
        }
        else
        {
            cout << "Error: cannot load image: " << filename << endl;
            return;
        }
    }

    return;
}


void printProcessingInfo(DepthConfigExtPtr depthConfigPtr) {

    OPTIMIZE_MODE optMode = depthConfigPtr->optimizeMode;
    DEPTH_REGISTRATION registerCam = depthConfigPtr->depthRegistration;
    double depthScale = depthConfigPtr->depthScale;
    double depthUnit  = depthConfigPtr->depthUnit;


    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Optimize Mode: %s", (optMode == 0) ? "QUALITY" : "SPEED or STD");
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Space  :    %d (0:L, 1:R, 2:RGB)", registerCam);
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Scale  : %.2f (0.00 ~ 1.00)", depthScale);
    TLog::log(TLog::LOG_ALWAYS, "App: ProcessingInfo: Depth Unit   : %.2f (0.02 / 0.05)\n\n", depthUnit);

    string depthCamStr = getDepthCamTypeString(depthConfigPtr->depthCameraType);
    int minZ = depthConfigPtr->minScanDistance;
    int maxZ = depthConfigPtr->maxScanDistance;

    TLog::log(TLog::LOG_ALWAYS, "ProcessingInfo: DepthCameraType:       %s", depthCamStr.c_str());
    TLog::log(TLog::LOG_ALWAYS, "ProcessingInfo: min/max scan distance: %d~%d mm\n\n", minZ, maxZ);


    //TLog::log(TLog::LOG_INFO, "ProcessingInfo: USE SIMD: %d", USE_SIMD);
}

std::string generateSuffixFromConfig(DepthConfigExtPtr depthConfigPtr) {

    string optstr;
    switch (depthConfigPtr->optimizeMode) {
        case OPTIMIZE_FULL_RES:  optstr = "_FULL";
            break;
        case OPTIMIZE_HALF_RES: optstr = "_HALF";
            break;
        case OPTIMIZE_QUARTER_RES:    optstr = "_Q";
            break;
        case OPTIMIZE_QQUARTER_RES:    optstr = "_QQ";
            break;
    }


    string IRthres = (depthConfigPtr->useIRThres ? "_IR" : "");
    string twoDisp = (depthConfigPtr->useTwoDispMaps ? "_TD" : "");
    string deRange = (depthConfigPtr->useDepthRange ? "_DR" : "");
    string reBias = (depthConfigPtr->reduceDispBias ? "_RB" : "");

    string configSuffix = optstr + IRthres + deRange + twoDisp + reBias;

    return configSuffix;
}

int compareDispQuality(const cv::Mat& disp1, const cv::Mat& disp2, const cv::Mat& disp3)
{
    if (disp1.size() != disp2.size() 
        || disp1.size() != disp3.size()
        || disp1.type() != disp2.type()
        || disp1.type() != disp3.type())
        return false;

    const int rows = disp1.rows;
    const int cols = disp1.cols;

    std::vector<ushort> gradientVals1, gradientVals2, gradientVals3;
    gradientVals1.reserve(disp1.cols * disp1.rows);
    gradientVals2.reserve(disp2.cols * disp2.rows);
    gradientVals3.reserve(disp3.cols * disp3.rows);

    float sum1 = 0, sum2 = 0, sum3 = 0;

    for (int r = 0; r != rows; ++r)
    {
        const ushort* dispPtr1 = disp1.ptr<ushort>(r, 0);
        const ushort* dispPtr2 = disp2.ptr<ushort>(r, 0);
        const ushort* dispPtr3 = disp3.ptr<ushort>(r, 0);

        for (int c = 0; c != cols - 1; ++c)
        {
            const ushort dispVal1 = *dispPtr1++;
            const ushort dispVal2 = *dispPtr2++;
            const ushort dispVal3 = *dispPtr3++;

            const ushort tmpVal1 = *dispPtr1;
            const ushort tmpVal2 = *dispPtr2;
            const ushort tmpVal3 = *dispPtr3;

            if (dispVal1 && tmpVal1)
            {
                const ushort gradient = dispVal1 > tmpVal1 ?
                    dispVal1 - tmpVal1 : tmpVal1 - dispVal1;
                sum1 += gradient;
                gradientVals1.push_back(gradient);
            }

            if (dispVal2 && tmpVal2)
            {
                const ushort gradient = dispVal2 > tmpVal2 ?
                    dispVal2 - tmpVal2 : tmpVal2 - dispVal2;
                sum2 += gradient;
                gradientVals2.push_back(gradient);
            }

            if (dispVal3 && tmpVal3)
            {
                const ushort gradient = dispVal3 > tmpVal3 ?
                    dispVal3 - tmpVal3 : tmpVal3 - dispVal3;
                sum3 += gradient;
                gradientVals3.push_back(gradient);
            }
        }
    }

    const float cnt1 = static_cast<float>(gradientVals1.size());
    const float cnt2 = static_cast<float>(gradientVals2.size());
    const float cnt3 = static_cast<float>(gradientVals3.size());

    const double differ = std::fabs(cnt1 - cnt2) / std::max(cnt1, cnt2);

    //if (differ > 0.2)
    //{
    //    return cnt1 > cnt2 ? true : false;
    //}

    const float avg1 = sum1 / cnt1, avg2 = sum2 / cnt2, avg3 = sum3 / cnt3;
    float rms1 = 0, rms2 = 0, rms3 = 0;
    for (const ushort& val : gradientVals1)
    {
        rms1 += (val - avg1) * (val - avg1);
    }
    rms1 = std::sqrtf(rms1 / cnt1);

    for (const ushort& val : gradientVals2)
    {
        rms2 += (val - avg2) * (val - avg2);
    }
    rms2 = std::sqrtf(rms2 / cnt2);

    for (const ushort& val : gradientVals3)
    {
        rms3 += (val - avg3) * (val - avg3);
    }
    rms3 = std::sqrtf(rms3 / cnt3);

    std::cout << "rms1: " << rms1 << std::endl;
    std::cout << "rms2: " << rms2 << std::endl;
    std::cout << "rms3: " << rms3 << std::endl;

    if (rms1 < rms2 && rms1 < rms3)
        return 1;
    else if (rms2 < rms1 && rms2 < rms3)
        return 2;
    else if (rms3 < rms1 && rms3 < rms2)
        return 3;
    else
        return 0;
}


string dir_arg, outfolder_arg;
double scale_arg;
float depthScale_arg, depthUnit_arg;
int optMode_arg, sceneMode_arg, depthSpace_arg;

int isTD_arg, isIR_arg, isDR_arg, isRB_arg;
int verbose_arg, writeobj_arg;
int block_arg, base_arg, minz_arg, maxz_arg;


int main(int argc, char** argv) {

    const String keys =
        "{help h       |       | print this message              }"
        "{@dir         |       | path to the data directory      }"
        "{verbose    b |   1   | Log level: 1-Verbose, 2-Info, 3-Warning, 4-Error }"
        "{sceneMode  n |   0   | Scene Mode: 0-Normal, 1-FlatWall, 2-Outdoor, 3-Indoor }"
        "{optMode    o |   0   | Optimization Mode: 0-Quality, 1-Standard, 2-Speed }"
        "{depthSpace d |   0   | Out depth space: 0-Lcam, 1-Rcam, 2-RGB-cam }"
        "{depthScale s | 1.00f | depth map scale: 0.0 ~ 1.0 (mostly used for RGB space) }"
        "{depthUnit  u | 0.02f | depth map unit:  0.02 covers 1.3 meters, 0.05 covers 3 meters }"
        "{isTD      td |   0   | use two disparity maps }"
        "{isDR      dr |   0   | use depth range estimate }"
        "{isRB      rb |   0   | reduce bias in disp map }"
        "{isIR      ir |   0   | use IR filtering }"
        "{writeObj   m |   0   | is output obj mesh: 0 - No, 1 - Yes }"
    ;

    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    else {
        dir_arg = parser.get<string>("@dir");

        optMode_arg    = parser.get<int>("optMode");
        sceneMode_arg  = parser.get<int>("sceneMode");
        depthSpace_arg = parser.get<int>("depthSpace");

        depthScale_arg = parser.get<float>("depthScale");
        depthUnit_arg  = parser.get<float>("depthUnit");

        writeobj_arg = parser.get<int>("writeObj");
        verbose_arg  = parser.get<int>("verbose");


        isTD_arg = parser.get<int>("isTD");
        isDR_arg = parser.get<int>("isDR");
        isRB_arg = parser.get<int>("isRB");
        isIR_arg = parser.get<int>("isIR");
    }

    help();

    TLog::setLogLevel((TLog::LogType)verbose_arg);

    outfolder_arg = "Results/";  // FaceResults  // WallResults

#if _DEBUG
    dir_arg = "C:/Users/JohnZ-Bellus3D/Documents/GitHub/b3d_server/data/20191008_frameSync/DIST7/";
    b3di::TLog::setLogFile((dir_arg + "computeDepth.log").c_str(), true);
    // 00_INU25_Tests  // 0_OFILM_25  // 000_PROTOOA4_Test // 000_PROTOOA4_FlatWall
    
    //sceneMode_arg = 4; // 4-FaceID, 5-Demo
    optMode_arg = 0;  // 0 - Quality, 1 - Standard, 2 - Speed

    isTD_arg = 1;
    isDR_arg = 1;
    isRB_arg = 1;
    isIR_arg = 0;

    writeobj_arg = 0;
#endif
    // ---- Read rectification maps
    // create a new calib data ptr and read calib.data file
    const std::string calibDataPath = dir_arg + "calib.data";
    b3dd::CameraCalibExtPtr cameraCalibExtPtr = std::make_shared<b3dd::CameraCalibExt>();
    
    bool ret = cameraCalibExtPtr->loadFromFile(calibDataPath);
    if (!ret || cameraCalibExtPtr->getCalibDataVersion() != CALIB_DATA_VERSION)
    {
        if (!ret)
        {
            TLog::log(TLog::LOG_WARNING, "Fail to load calib data file %s", (calibDataPath).c_str());
        }
        else if (cameraCalibExtPtr->getCalibDataVersion() != CALIB_DATA_VERSION)
        {
            TLog::log(TLog::LOG_WARNING, "Calib data version [%.1f] -- Camera Calib version [%.1f]", 
                cameraCalibExtPtr->getCalibDataVersion(), CALIB_DATA_VERSION);
            TLog::log(TLog::LOG_WARNING, "Create calib data file from calib bin file");
        }

        const std::string calibBinFileName = "b3dCalibData.bin";
        B3DCalibrationData calibBin;
        ret = readB3DCalibrationData(dir_arg, calibBinFileName, calibBin);

        if (!readB3DCalibrationData(dir_arg, calibBinFileName, calibBin))
        {
            TLog::log(TLog::LOG_ERROR, "Fail to load calib bin file %s", (dir_arg + calibBinFileName).c_str());
            return -1;
        }

        const B3DCalibrationData* calibBinPtr = &calibBin;

        cameraCalibExtPtr->cacheCalibrationData(
            (unsigned char*)calibBinPtr,
            calibDataPath,
            DepthCameraType::DEPTHCAM_B3D4
        );

        if (!cameraCalibExtPtr->loadFromFile(calibDataPath))
        {
            TLog::log(TLog::LOG_ERROR, "Fail to load calib data file %s", (calibDataPath).c_str());
            return -1;
        }
    }

    // Prepare output directory
    string outputDir = dir_arg + outfolder_arg;
    b3di::FileUtils::createDir(outputDir);

    // Configure DepthProcessor settings
    DepthProcessorExt depthProcessor;
    depthProcessor.loadCalibrationData(cameraCalibExtPtr);

    // ---- Parse cmd line arguments
    DepthConfigExtPtr depthConfigPtr  = shared_ptr<DepthConfigExt>(new DepthConfigExt());
    depthConfigPtr->sceneMode         = (SCENE_MODE)sceneMode_arg;
    depthConfigPtr->depthRegistration = (DEPTH_REGISTRATION)depthSpace_arg;
    depthConfigPtr->depthUnit         = depthUnit_arg;

    // EXT settings
    depthConfigPtr->useTwoDispMaps = (isTD_arg == 1);
    depthConfigPtr->useDepthRange  = (isDR_arg == 1);
    depthConfigPtr->reduceDispBias = (isRB_arg == 1);
    depthConfigPtr->useIRThres     = (isIR_arg == 1);
    //depthConfigPtr->ROI_L = ROI(180, 230, 100, 200);

    // ---- Read raw IR images

    vector<B3DImage> imagesL, imagesR;
    vector<cv::String> imageNamesL, imageNamesR;
    getFilePaths(dir_arg + "L/", "*.png", imageNamesL);
    getFilePaths(dir_arg + "R/", "*.png", imageNamesR);

    std::size_t imageCount = imageNamesL.size();
    vector<cv::String> imageNames = imageNamesL;

    // Make sure depth configurations are actually used in depth processor
    // in case user forgot to use "updateProcessorSettings", it will print DEFAULT settings value

    TProfile tpCompuDepth;
    double totalTime = 0.0;

    std::vector<OPTIMIZE_MODE> optimizeModes{
        OPTIMIZE_MODE::OPTIMIZE_FULL_RES,
        OPTIMIZE_MODE::OPTIMIZE_HALF_RES,
        OPTIMIZE_MODE::OPTIMIZE_QUARTER_RES
    };

    for (int i = 0; i < imageCount; ++i) {

        TLog::log(TLog::LOG_ALWAYS, "Processing <%s> ... \n", imageNames[i].c_str());

        depthConfigPtr->optimizeMode = optimizeModes[i % optimizeModes.size()];

        if (i == 0)
            depthConfigPtr->optimizeMode = optimizeModes[i % optimizeModes.size()];
        else
            depthConfigPtr->optimizeMode = optimizeModes[i % 2 + 1];

        const float imageScale = depthConfigPtr->optimizeMode == OPTIMIZE_MODE::OPTIMIZE_FULL_RES ? 1.0f :
            depthConfigPtr->optimizeMode == OPTIMIZE_MODE::OPTIMIZE_HALF_RES ? 0.75f :
            depthConfigPtr->optimizeMode == OPTIMIZE_MODE::OPTIMIZE_QUARTER_RES ? 0.5f : 0.25f;

        depthConfigPtr->depthScale = imageScale;

        // update depth processor settings
        depthProcessor.updateProcessorSettingsExt(depthConfigPtr);

        printProcessingInfo(depthProcessor.getProcessorSettingsExt());

        // load images
        B3DImage imageL, imageR;
        if (!loadB3DImage(dir_arg + "L/", imageL, imageNamesL[i], imageScale)
            || !loadB3DImage(dir_arg + "R/", imageR, imageNamesR[i], imageScale))
        {
            std::cout << "Cannot read file " + dir_arg + "L/" + imageNamesL[i] 
                << std::endl;
            std::cout << "Cannot read file " + dir_arg + "R/" + imageNamesR[i]
                << std::endl;
            return -1;
        }

        B3DImage outDepthMap, outDispMap;
        bool computeDepthSuccess = false;

        double t1 = now_ms();

        computeDepthSuccess = depthProcessor.computeDepth(imageL, imageR, outDepthMap);
        //computeDepthSuccess = depthProcessor.computeDisparity(imagesL[i], imagesR[i], outDispMap);

        totalTime += (now_ms() - t1);

        if (!computeDepthSuccess) {
            TLog::log(TLog::LOG_ERROR, "Fail to process <%s> ... \n", imageNames[i].c_str());
            continue;
        }

        // Construct output filename:
        int len = (int)imageNames[i].length();
        string fname = imageNames[i].substr(0, len - 4/*= suffix length (.png)*/);
        string configSuffix = generateSuffixFromConfig(depthProcessor.getProcessorSettingsExt());

        // ---- Output depth map ( .png & .obj )
        B3DImage showDepth;

        //convert16To8(outDepthMap, showDepth);
        //saveB3DImage(outputDir + fname + ".grayscale" + configSuffix + ".png", showDepth);

        convert16UToHeatmap(outDepthMap, showDepth, /*isDynamic*/ false);

        //convert16UToNormalmap(outDepthMap, showDepth, /*isC4*/ false);

        saveB3DImage(outputDir + fname + ".color" + configSuffix + ".png", showDepth);
        
        // ---- Save 16bit Depth map
        cv::Mat depthMap;
        B3DImageToCvMat(outDepthMap, depthMap);

        cv::resize(depthMap, depthMap, cv::Size(), 
            1.0 / imageScale, 1.0 / imageScale, cv::INTER_LINEAR);

        cv::imwrite(outputDir + "D" + fname.substr(1, fname.size() - 1) + ".png", depthMap);

        //// ---- Save 16bit Disp map
        //cv::Mat dispMap;
        //
        //B3DImageToCvMat(outDispMap, dispMap, B3DImageType::B3DIMAGE_DEPTH_16);
        //cv::imwrite(outputDir + "Disp" + fname.substr(1, fname.size() - 1) + ".png", dispMap);

        //cv::Mat showDepthMat(showDepth.rows(), showDepth.cols(), CV_8UC3, showDepth.data());
        //cv::imshow("Depth Demo", showDepthMat);
        //cv::waitKey(30);

        //bool debugIRMask = true;
        //if (debugIRMask) {
        //    B3DImage outIRMask;
        //    depthProcessor.computeIRMask(imagesSTEREO[i], outIRMask);
        //    saveB3DImage(outputDir + fname + ".mask" + configSuffix + ".png", outIRMask);
        //}


        // Output .OBJ mesh file
        if (writeobj_arg) {

            DepthToObj_Input input;
            input.outputDirPath = outputDir;
            input.objFileName = fname + "." + configSuffix + ".obj";

            string calibFilename = (depthConfigPtr->depthRegistration == REGISTER_L) ? "leftCam.yml" : "midCam.yml";
            string camPath = dir_arg + "CalibrationFiles/" + calibFilename;
            input.depthCam.loadParams(camPath);

            cv::Mat depthMat(outDepthMap.rows(), outDepthMap.cols(), CV_16UC1, outDepthMap.data());

            imwrite(outputDir + fname + ".depth" + configSuffix + ".png", depthMat);

            input.depthImage = depthMat;
            //cv::imwrite(outputDir + fname + ".depth" + config + ".png", depthMat);
            //std::cout << "!!!!! write depth " + outputDir + fname + ".depth" + config + ".png" << std::endl;

            // FIXME:  How shall we determine these two values?
            input.minZ = 100.0f;
            input.maxZ = 2000.0f;

            //// Output depth map in png
            //imwrite(outputDir + fname + ".depth." + config + ".png", depthMat);

            b3di::g_depthUnitConverter.setUnit(depthProcessor.getProcessorSettingsExt()->depthUnit);

            DepthToObj_Output output;
            int ret = depthToObj(input, output);
            if (ret < 0) {
                TLog::log(TLog::LOG_ERROR, "depthToObj returns error");
            }
        }
        cout << endl;
    }  // End of looping through images

    depthProcessor.printTimingStatistics();

    while (true);

    return 0;

}
