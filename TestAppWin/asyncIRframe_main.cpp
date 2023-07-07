#include <string>
#include <vector>
#include <Windows.h>
#include <String>
#include <algorithm>

#include "opencv2/core.hpp"
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "b3di/FileUtils.h"

#include <b3ddepth/utils/TLog.h>

using namespace std;
using namespace b3dd;
using namespace cv;

typedef std::pair<long long, cv::Mat> FrameInfo;
typedef std::pair<FrameInfo, FrameInfo> IRFramePair;
const long NSTOMS = 1000000L;

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

int main(int argc, const char* argv[]) {

    b3dd::TLog::setLogLevel(TLog::LOG_INFO);

    std::string dirPath = "";
    vector<std::string> filenamesL;
    vector<std::string> filenamesR;
    vector<IRFramePair> frameContainer;
    vector<IRFramePair> newframeContainer;

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

    filenamesL = get_all_files_names_within_folder(dirPath + "L/");
    filenamesR = get_all_files_names_within_folder(dirPath + "R/");

    logVerbose("after sorted....");
    for (int x = 0; x < filenamesL.size(); x++)
        logVerbose( " L : %s , R : %s", filenamesL[x].c_str(), filenamesR[x].c_str());

    for (int x = 0; x < filenamesL.size(); x++) {
        std::string file = "";  
        long long timeStamp = 0LL;
        long frameid = 0L;
        FrameInfo frameL, frameR;

        // L 
        logVerbose("file name L %s", filenamesL[x].c_str());
        file = dirPath + "L/" + filenamesL[x];
        cv::Mat frame = imread(file, CV_LOAD_IMAGE_GRAYSCALE);
        parseFrameName(filenamesL[x], frameid, timeStamp);
        frameL = std::make_pair(timeStamp, frame.clone());
        logVerbose("%lld %lld", timeStamp, frameL.first);


        // R 
        logVerbose("file name R %s", filenamesR[x].c_str());
        file.clear();
        file = dirPath + "R/" + filenamesR[x];
        frame = imread(file, CV_LOAD_IMAGE_GRAYSCALE);
        parseFrameName(filenamesR[x], frameid, timeStamp);
        frameR = std::make_pair(timeStamp, frame.clone());

        // push back
        frameContainer.push_back(std::make_pair(frameL, frameR));
        logVerbose("%lld %lld", frameContainer[x].first.first, frameContainer[x].second.first);

    }

    vector<IRFramePair> frameContainerbuffer;
    logInfo(" before replace frameContainer size %d", frameContainer.size());
    for (int x = 0; x < frameContainer.size(); x++) {
        long tL = frameContainer[x].first.first / 1000000LL;
        long tR = frameContainer[x].second.first / 1000000LL;
        long td = tL - tR;
        logInfo("tL %lld , tR %lld, td %lld",tL, tR, td);
        if (abs(td) > 100 && !frameContainerbuffer.empty()) {
            logInfo("detect timestamp not sync, start replace buffer work!");
            // poll and clean prev buffer from queue
            IRFramePair temp = frameContainerbuffer[0];
            // copy current buffer to container
            frameContainerbuffer.clear(); //make sure only have one index in vector
            frameContainerbuffer.push_back(frameContainer[x]);
            if (td > 100) {
                logInfo("timeStampL-timeStampR > 100");
                // replace L buffer from previous
                IRFramePair newBuffer = std::make_pair(temp.first, frameContainer[x].second);
                newframeContainer.push_back(newBuffer);
            }
            else if (td <= 100) {
                logInfo("timeStampL-timeStampR <= 100");
                // replace R buffer from previous
                IRFramePair newBuffer = std::make_pair(frameContainer[x].first, temp.second);
                newframeContainer.push_back(newBuffer);
            }
        }
        // we got a first async frame
        else if (abs(td) > 100 && frameContainerbuffer.empty()) {
            logInfo("detect timestamp not sync and previous cache is empty!");
            frameContainerbuffer.push_back(frameContainer[x]);
        }
        else if (!frameContainerbuffer.empty()) {
            logInfo("make sure byteDataPrevious empty!");
            // reset frameContainerbuffer , so it can restart again
            newframeContainer.push_back(frameContainer[x]);
            frameContainerbuffer.clear();
        }

    }

    logInfo(" after replace frameContainer size %d", newframeContainer.size());

    // prepare folder
    std::string folder = dirPath + "new";
    b3di::FileUtils::createDir(folder);
    folder = dirPath + "new/L";
    b3di::FileUtils::createDir(folder);
    folder = dirPath + "new/R";
    b3di::FileUtils::createDir(folder);

    for (int x = 0; x < newframeContainer.size(); x++) {
        std::string file;
        stringstream ssL,ssR;

        // L 
        ssL << setw(3) << setfill('0') << to_string(x+1);
        std::string frameindex = ssL.str() + "_";
        std::stringstream().swap(ssL);

        ssL << setw(6) << setfill('0') << to_string(newframeContainer[x].first.first);
        std::string timeStamp = ssL.str() + "_";
        std::stringstream().swap(ssL);

        ssL << setw(5) << setfill('0') << to_string(x + 1);
        std::string frameindex_2 = ssL.str();

        file = dirPath + "new/L/" + frameindex + timeStamp + frameindex_2 + ".png";
        imwrite(file, newframeContainer[x].first.second);

        frameindex.clear();
        timeStamp.clear();
        frameindex_2.clear();
        file.clear();

        // R 
        ssR << setw(3) << setfill('0') << to_string(x + 1);
        frameindex = ssR.str() + "_";
        std::stringstream().swap(ssR);

        ssR << setw(6) << setfill('0') << to_string(newframeContainer[x].second.first);
        timeStamp = ssR.str() + "_";
        std::stringstream().swap(ssR);

        ssR << setw(5) << setfill('0') << to_string(x + 1);
        frameindex_2 = ssR.str();

        file = dirPath + "new/R/" + frameindex + timeStamp + frameindex_2 + ".png";
        imwrite(file, newframeContainer[x].second.second);
    }
    return 0;
}