#include <string>
#include <vector>
#include <Windows.h>
#include <String>
#include <algorithm>
#include <iostream>

#include "opencv2/core.hpp"
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "b3di/FileUtils.h"

#include <b3ddepth/utils/TLog.h>


using namespace std;
using namespace b3dd;
using namespace cv;

typedef std::pair<long long, cv::Mat> FrameInfo; // <timeStamp, image buffer>
const long NSTOMS = 1000000L;

enum CAM_POS {
    POS_C,
    POS_l1,
    POS_r1,
    POS_l2,
    POS_r2,
    POS_u1,
    POS_d1,
    POS_END
};

vector<string> posFolder = {
    "c",
    "l1",
    "r1",
    "l2",
    "r2",
    "u1",
    "d1"
};

int parseFrameName(const string& filename, long& frameId, long long& timeStamp) {
    int firstBottonLine = filename.find_first_of('_');
    int secondBottonLine = (filename.substr(firstBottonLine + 1, filename.length())).find_first_of('_');
    int firstdot = filename.find_first_of('.');
    string s_frameid = filename.substr(firstBottonLine + 1, secondBottonLine);
    string s_timeStamp = filename.substr(firstBottonLine + secondBottonLine + 2, firstdot - (firstBottonLine + secondBottonLine + 2));
    logVerbose("frame id %s, timeStamp %s", s_frameid.c_str(), s_timeStamp.c_str());
    frameId = atol(s_frameid.c_str());
    timeStamp = stoll(s_timeStamp, nullptr, 10);
    return 1;
}

struct myCom {
    bool operator() (const std::string s1, const std::string s2) const {
        int firstBottonLine1 = s1.find_first_of('_');
        int secondBottonLine1 = (s1.substr(firstBottonLine1 + 1, s1.length())).find_first_of('_');
        string s_frameid1 = s1.substr(firstBottonLine1 + 1, secondBottonLine1);
        int firstBottonLine2 = s2.find_first_of('_');
        int secondBottonLine2 = (s2.substr(firstBottonLine2 + 1, s2.length())).find_first_of('_');
        string s_frameid2 = s2.substr(firstBottonLine2 + 1, secondBottonLine2);
        logVerbose("%s %s", s_frameid1.c_str(), s_frameid2.c_str());
        return stoi(s_frameid1) < stoi(s_frameid2);
    }
};

std::string getFilenamePrefix(std::string input) {

    std::size_t found = input.find('.');
    return input.substr(0, found);
}

vector<std::string> get_all_files_names_within_folder(std::string folder)
{
    vector<std::string> names;
    std::string search_path = b3di::FileUtils::appendPath(folder, "*.*");
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                names.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }

    sort(names.begin(), names.end(), myCom());

    return names;
}

char* readFileBytes(const char *name)
{
    ifstream fl(name, ios::in | ios::binary);
    fl.seekg(0, ios::end);
    size_t len = fl.tellg();
    if (len == 0) {
        logError("File %s is empty.", name);
        return NULL;
    }
    char *ret = new char[len];
    fl.seekg(0, ios::beg);
    fl.read(ret, len);
    fl.close();
    logVerbose("File lens : %d", len);
    return ret;
}

int main(int argc, const char* argv[]) {

    b3dd::TLog::setLogLevel(TLog::LOG_INFO);

    std::string dirPath = "";
    // vector store all position's filename
    vector<vector<std::string>> filenamesL;
    vector<vector<std::string>> filenamesR;
    vector<vector<std::string>> filenamesM;
    vector<vector<std::string>> filenamesD;

    // vector stor all position's frame
    vector<vector<FrameInfo>> frameContainerL(CAM_POS::POS_END, vector<FrameInfo>());
    vector<vector<FrameInfo>> frameContainerR(CAM_POS::POS_END, vector<FrameInfo>());
    vector<vector<FrameInfo>> frameContainerM(CAM_POS::POS_END, vector<FrameInfo>());
    vector<vector<FrameInfo>> frameContainerD(CAM_POS::POS_END, vector<FrameInfo>());
    vector<long> offset = { -1,-1,-1,-1,-1,-1,-1 };
    vector<long> startFrame = { -1, -1, -1, -1, -1, -1, -1 };

    const cv::String keys =
        "{help h       |       | print this message              }"
        "{@dir         |       | path to the data directory      }"
        ;

    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    else {
        dirPath = parser.get<String>("@dir");
    }

    // calculate offset
    // read json file
    const std::string BUFFER_OK = "time_offset.json";
    const std::string NOTIFI = "notification_time_offset.json";
    string timeOffsetPath = dirPath + NOTIFI;
    cv::FileStorage fs(timeOffsetPath, cv::FileStorage::READ);
    if (!fs["c"].empty())  offset[CAM_POS::POS_C] =  stol(fs["c"]);
    if (!fs["l1"].empty()) offset[CAM_POS::POS_l1] = stol(fs["l1"]);
    if (!fs["r1"].empty()) offset[CAM_POS::POS_r1] = stol(fs["r1"]);
    if (!fs["l2"].empty()) offset[CAM_POS::POS_l2] = stol(fs["l2"]);
    if (!fs["r2"].empty()) offset[CAM_POS::POS_r2] = stol(fs["r2"]);
    if (!fs["d1"].empty()) offset[CAM_POS::POS_d1] = stol(fs["d1"]);
    if (!fs["u1"].empty()) offset[CAM_POS::POS_u1] = stol(fs["u1"]);
    fs.release();

    for (int x = 0; x < offset.size(); x++)
        logInfo("pose %s, offset %ld", posFolder[x].c_str(), offset[x]);

#ifdef _DEBUG
    offset[CAM_POS::POS_C]  = 3537;
    offset[CAM_POS::POS_l1] = 3546;
    offset[CAM_POS::POS_r1] = 3506;
    offset[CAM_POS::POS_d1] = 3602;
#endif
    int minPos = -1;
    long minTimeStamp = LONG_MAX;
    for (int x = 0; x < offset.size(); x++) {
        if (offset[x] != -1 && offset[x] < minTimeStamp) {
            minTimeStamp = offset[x];
            minPos = x;
        }
    }
    logInfo("base cam is %s", posFolder[minPos].c_str());
    long base = offset[minPos];
    startFrame[minPos] = 0;

    for (int x = 0; x < offset.size(); x++) {
        if (offset[x] != -1) {
            offset[x] -= base;
        }
        logInfo("pose %s, normalized %ld", posFolder[x].c_str(), offset[x]);
    }

    // get all file names
    for (int x = 0; x < CAM_POS::POS_END; x++) {
        string path = dirPath + "/" + posFolder[x] + "/4D/L/";
        filenamesL.push_back(get_all_files_names_within_folder(path));
        path = dirPath + "/" + posFolder[x] + "/4D/R/";
        filenamesR.push_back(get_all_files_names_within_folder(path));
        path = dirPath + "/" + posFolder[x] + "/4D/M/";
        filenamesM.push_back(get_all_files_names_within_folder(path));
        path = dirPath + "/" + posFolder[x] + "/4D/D/";
        filenamesD.push_back(get_all_files_names_within_folder(path));
    }


    for (int camPose = 0; camPose < filenamesM.size(); camPose++) {
        if (filenamesM[camPose].empty()) continue;
        logInfo("parsing position %s", posFolder[camPose].c_str());
        for (int frames = 0; frames < filenamesM[camPose].size(); frames++) {

            std::string file = "";
            long long timeStamp = 0LL;
            long frameid = 0L;
            FrameInfo frameL, frameR, frameM, frameD;
            unsigned char *buffer;
            cv::Mat frame;
#if 0
            // parse L if not empty
            logVerbose("file name L %s", filenamesL[camPose][frames].c_str());
            file = dirPath + posFolder[camPose] + "/4D/L/" + filenamesL[camPose][frames];
            buffer = (unsigned char*)readFileBytes(file.c_str());
            cv::Mat frame = cv::Mat::zeros(1280, 800, CV_8UC1);
            memcpy(frame.data, buffer, 800*1280 * sizeof(unsigned char));
            parseFrameName(filenamesL[camPose][frames], frameid, timeStamp);
            frameL = std::make_pair(timeStamp, frame.clone());
            logVerbose("%lld %lld", timeStamp, frameL.first);

            frameContainerL[camPose].push_back(frameL);

            // parse R if not empty
            logVerbose("file name R %s", filenamesR[camPose][frames].c_str());
            file = dirPath + posFolder[camPose] + "/4D/R/" + filenamesR[camPose][frames];
            buffer = (unsigned char*)readFileBytes(file.c_str());
            frame = cv::Mat::zeros(1280, 800, CV_8UC1);
            memcpy(frame.data, buffer, 800 * 1280 * sizeof(unsigned char));
            parseFrameName(filenamesR[camPose][frames], frameid, timeStamp);
            frameR = std::make_pair(timeStamp, frame.clone());
            logVerbose("%lld %lld", timeStamp, frameR.first);

            frameContainerR[camPose].push_back(frameR);
#endif
            // parse M if not empty
            logVerbose("file name M %s", filenamesM[camPose][frames].c_str());
            file = dirPath + "/" + posFolder[camPose] + "/4D/M/" + filenamesM[camPose][frames];
            buffer = (unsigned char*)readFileBytes(file.c_str());
            frame = cv::Mat::zeros(1632 * 3 / 2, 1224, CV_8UC1);
            cv::Mat frameRGB;
            memcpy(frame.data, (unsigned char *)buffer,
                (1632*1224*3/2) * sizeof(unsigned char));
            cv::cvtColor(frame, frameRGB, CV_YUV2BGR_NV21);
            parseFrameName(filenamesM[camPose][frames], frameid, timeStamp);
            frameM = std::make_pair(timeStamp, frameRGB.clone());
            logVerbose("%lld %lld", timeStamp, frameM.first);

            frameContainerM[camPose].push_back(frameM);

            // parse D if not empty
            logVerbose("file name M %s", filenamesD[camPose][frames].c_str());
            file = dirPath + "/" + posFolder[camPose] + "/4D/D/" + filenamesD[camPose][frames];
            buffer = (unsigned char*)readFileBytes(file.c_str());
            frame = cv::Mat::zeros(960, 600, CV_16UC1);
            memcpy(frame.data, buffer, 800 * 1280 * sizeof(unsigned char));
            parseFrameName(filenamesD[camPose][frames], frameid, timeStamp);
            frameD = std::make_pair(timeStamp, frame.clone());
            logVerbose("%lld %lld", timeStamp, frameD.first);

            frameContainerD[camPose].push_back(frameD);
        }
    }

    logInfo("calculating first frame...");
    // calculate first frame index
    for (int camPose = 0; camPose < filenamesM.size(); camPose++) {
        // if offset < 50 we set the start index is 0, since our target is 10fps
        // so the interval of frame is 100ms
        if (offset[camPose] != -1 && offset[camPose] < 50) {
            startFrame[camPose] = 0;
        } else if (offset[camPose] != -1 && startFrame[camPose] != 0) {
            long curFirstTimeStamp = frameContainerM[camPose][0].first;
            long timeStampToFind = curFirstTimeStamp + offset[camPose];
            int diffSec = abs(frameContainerM[camPose][1].first - timeStampToFind);
            int diffThird = abs(frameContainerM[camPose][2].first - timeStampToFind);
            logInfo("cam %s, diff with sec frame %d, diff with third frame %d", posFolder[camPose].c_str(), diffSec, diffThird);
            if (diffSec >= diffThird) startFrame[camPose] = 2;
            else startFrame[camPose] = 1;
        }
        if(offset[camPose] != -1)
            logInfo("cam %s, offset with base cam is %d ms, start frame is %d", posFolder[camPose].c_str(), offset[camPose], startFrame[camPose]);
    }


    logInfo("saving original frames...");
    // save original frame for debug
    if (!b3di::FileUtils::directoryExists(dirPath + "original")) b3di::FileUtils::createDir(dirPath + "original");

    for (int camPose = 0; camPose < filenamesM.size(); camPose++) {
        for (int frames = 0; frames < filenamesM[camPose].size(); frames++) {

            std::string camp = dirPath + "/original/" + posFolder[camPose];
            if (!b3di::FileUtils::directoryExists(camp)) b3di::FileUtils::createDir(camp);

            std::string file, folder, frameindex, frameindex_2, timeStamp;
            stringstream ssL, ssR, ssM, ssD;
#if 0
            std::string folder = dirPath + "/original/" + posFolder[camPose] + "/L/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // L 
            ssL << setw(3) << setfill('0') << to_string(frames + 1);
            std::string frameindex = ssL.str() + "_";
            std::stringstream().swap(ssL);

            ssL << setw(6) << setfill('0') << to_string(frameContainerL[camPose][frames].first);
            std::string timeStamp = ssL.str() + "_";
            std::stringstream().swap(ssL);

            ssL << setw(5) << setfill('0') << to_string(frames + 1);
            std::string frameindex_2 = ssL.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerL[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#endif
            folder = dirPath + "/original/" + posFolder[camPose] + "/M/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // M 
            ssM << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssM.str() + "_";
            std::stringstream().swap(ssM);

            ssM << setw(6) << setfill('0') << to_string(frameContainerM[camPose][frames].first);
            timeStamp = ssM.str() + "_";
            std::stringstream().swap(ssM);

            ssL << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssM.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".jpg";
            imwrite(file, frameContainerM[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#if 0
            folder = dirPath + "/original/" + posFolder[camPose] + "/R/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // R 
            ssR << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssR.str() + "_";
            std::stringstream().swap(ssR);

            ssR << setw(6) << setfill('0') << to_string(frameContainerR[camPose][frames].first);
            timeStamp = ssR.str() + "_";
            std::stringstream().swap(ssR);

            ssR << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssR.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerR[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#endif
            folder = dirPath + "/original/" + posFolder[camPose] + "/D/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // M 
            ssD << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssD.str() + "_";
            std::stringstream().swap(ssD);

            ssD << setw(6) << setfill('0') << to_string(frameContainerD[camPose][frames].first);
            timeStamp = ssD.str() + "_";
            std::stringstream().swap(ssD);

            ssD << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssD.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerD[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
        }
    }


    logInfo("saving offset frames...");
    // save offset frames
    if (!b3di::FileUtils::directoryExists(dirPath + "Synced")) b3di::FileUtils::createDir(dirPath + "Synced");

    for (int camPose = 0; camPose < filenamesM.size(); camPose++) {
        int startIdx = startFrame[camPose];
        logInfo("camPose %s, start with frame %d (skip if start with negative frame)", posFolder[camPose].c_str(), startFrame[camPose]);
        if (startIdx < 0) continue;
        for (int frames = startIdx; frames < filenamesM[camPose].size(); frames++) {

            std::string camp = dirPath + "/Synced/" + posFolder[camPose];
            if (!b3di::FileUtils::directoryExists(camp)) b3di::FileUtils::createDir(camp);

            std::string file, folder, frameindex, frameindex_2, timeStamp;
            stringstream ssL, ssR, ssM, ssD;
#if 0
            std::string folder = dirPath + "/new/" + posFolder[camPose] + "/L/";
            if(!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // L 
            ssL << setw(3) << setfill('0') << to_string(frames + 1);
            std::string frameindex = ssL.str() + "_";
            std::stringstream().swap(ssL);

            ssL << setw(6) << setfill('0') << to_string(frameContainerL[camPose][frames].first);
            std::string timeStamp = ssL.str() + "_";
            std::stringstream().swap(ssL);

            ssL << setw(5) << setfill('0') << to_string(frames + 1);
            std::string frameindex_2 = ssL.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerL[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#endif
            folder = dirPath + "/Synced/" + posFolder[camPose] + "/M/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // M 
            ssM << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssM.str() + "_";
            std::stringstream().swap(ssM);

            ssM << setw(6) << setfill('0') << to_string(frameContainerM[camPose][frames].first);
            timeStamp = ssM.str() + "_";
            std::stringstream().swap(ssM);

            ssL << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssM.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".jpg";
            imwrite(file, frameContainerM[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#if 0
            folder = dirPath + "/new/" + posFolder[camPose] + "/R/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // R 
            ssR << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssR.str() + "_";
            std::stringstream().swap(ssR);

            ssR << setw(6) << setfill('0') << to_string(frameContainerR[camPose][frames].first);
            timeStamp = ssR.str() + "_";
            std::stringstream().swap(ssR);

            ssR << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssR.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerR[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
#endif
            folder = dirPath + "/Synced/" + posFolder[camPose] + "/D/";
            if (!b3di::FileUtils::directoryExists(folder)) b3di::FileUtils::createDir(folder);

            // M 
            ssD << setw(3) << setfill('0') << to_string(frames + 1);
            frameindex = ssD.str() + "_";
            std::stringstream().swap(ssD);

            ssD << setw(6) << setfill('0') << to_string(frameContainerD[camPose][frames].first);
            timeStamp = ssD.str() + "_";
            std::stringstream().swap(ssD);

            ssD << setw(5) << setfill('0') << to_string(frames + 1);
            frameindex_2 = ssD.str();

            file = folder + frameindex + timeStamp + frameindex_2 + ".png";
            imwrite(file, frameContainerD[camPose][frames].second);

            frameindex.clear();
            timeStamp.clear();
            frameindex_2.clear();
            file.clear();
        }
    }

    return 0;
}