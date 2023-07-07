#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <set>
#include <thread>

#include <Windows.h>

using namespace std;

const std::vector<std::string> FAC_CALIB_OUTPUT_FILES{
    "leftCam.yml",
    "rightCam.yml",
    "midCam.yml",
    "b3dCalibData.bin"
};

const std::string FAC_CALIB_FLASH_TARGET_FOLDER = "/mnt/vendor/CalibrationFiles/";

typedef enum
{
    LOG_ALL = 0,
    LOG_VERBOSE = 1,
    LOG_INFO = 2,
    LOG_WARNING = 3,
    LOG_ERROR = 4,
    LOG_ALWAYS = 5,			// Force to always print unless logLevel is LOG_NONE
    LOG_NONE = 10,
} LogType;

static std::string s_logTypeStrings[10] = {
    "",
    "VERBOSE",
    "INFO",
    "WARNING",
    "ERROR",
    ">>>",
    ">>>",
    ">>>",
    ">>>",
    ">>>"
};

static std::string logPath="";
LogType s_logType = LOG_ALL;

void log(LogType type, const char* strFormat, ...)
{
    bool logToFile = logPath.empty() ? false : true;
    if (logToFile || (type >= s_logType && type >= LOG_VERBOSE && type < LOG_NONE)) {
        char strBuf[1024];

        va_list args;
        va_start(args, strFormat);
        vsprintf(strBuf, strFormat, args);
        //vprintf_s(strFormat, args);
        va_end(args);

        int len = (int)strlen(strBuf);
        if (len == 0 || (len>0 && strBuf[len - 1] != '\n'))
            strcat(strBuf, "\n");

        if (type >= s_logType && type >= LOG_VERBOSE && type < LOG_NONE) {

            // Log to console

#if defined(WIN32) || defined(TARGET_OS_IPHONE)

            // Print error log to stderr
            if (type == LOG_ERROR) {
                fprintf(stderr, "%s %s", s_logTypeStrings[type].c_str(), strBuf);
            }
            else {
                // Print all others to stdout
                printf("%s %s", s_logTypeStrings[type].c_str(), strBuf);
            }

#elif defined(ANDROID)

            int androidLogType = ANDROID_LOG_VERBOSE;
            if (type == TLog::LOG_VERBOSE) {
                androidLogType = ANDROID_LOG_VERBOSE;
            }
            else if (type == TLog::LOG_INFO) {
                androidLogType = ANDROID_LOG_INFO;
            }
            else if (type == TLog::LOG_WARNING) {
                androidLogType = ANDROID_LOG_WARN;
            }
            else if (type == TLog::LOG_ERROR) {
                androidLogType = ANDROID_LOG_ERROR;
            }
            else if (type == TLog::LOG_ALWAYS) {
                androidLogType = ANDROID_LOG_ERROR;
            }
            __android_log_print(androidLogType, s_logTypeStrings[type].c_str(), strBuf, "");
#else
            // ERROR!!! but how do I print error?  this is the log function...
#endif

        }  // End of Log Filtering Control


           // Start Logging to File
           // Log only non-verbose messages unless s_logType is set to log verbose messages
        if (logToFile && (type >= s_logType || type>LOG_VERBOSE) && type < LOG_NONE) {
            FILE* fp = fopen(logPath.c_str(), "ab");
            if (fp) {
                char timeStr[60];
                time_t rawtime;
                struct tm* timeinfo;
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(timeStr, sizeof timeStr, "%c", timeinfo);


#if defined(WIN32) || defined(TARGET_OS_IPHONE)

                fprintf(fp, "%s %s %s\r\n", timeStr, s_logTypeStrings[type].c_str(), strBuf);

#elif defined(ANDROID)
                fprintf(fp, "%s %s %s\n", timeStr, s_logTypeStrings[type].c_str(), strBuf);
#else
                // ERROR!!! but how do I print error?  this is the log function...
#endif
                fclose(fp);
            }
        }
    }
}

#if defined(WIN32) || defined(TARGET_OS_IPHONE)

#define logAll(format, ...)                    \
        log(LOG_ALL, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logVerbose(format, ...)                \
        log(LOG_VERBOSE, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logInfo(format, ...)                   \
        log(LOG_INFO, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logWarning(format, ...)                \
        log(LOG_WARNING, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logError(format, ...)                  \
        log(LOG_ERROR, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#define logAlways(format, ...)                 \
        log(LOG_ALWAYS, "[%s] [%d] " format, __func__, __LINE__,__VA_ARGS__)

#elif defined(ANDROID)

#define logAll(fmt, args...)                    \
        b3dd::TLog::log(b3dd::TLog::LOG_ALL, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#define logVerbose(fmt, args...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_VERBOSE, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#define logInfo(fmt, args...)                   \
        b3dd::TLog::log(b3dd::TLog::LOG_INFO, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#define logWarning(fmt, args...)                \
        b3dd::TLog::log(b3dd::TLog::LOG_WARNING, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#define logError(fmt, args...)                  \
        b3dd::TLog::log(b3dd::TLog::LOG_ERROR, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#define logAlways(fmt, args...)                 \
        b3dd::TLog::log(b3dd::TLog::LOG_ALWAYS, "[%s] [%d] " fmt, __func__, __LINE__,##args)

#else
#endif

// excute console commands
int execute(const std::string& cmd, std::string& result)
{
    const std::string file_name = "tmp";
    int success = std::system((cmd + " 1> " + file_name + " 2>&1").c_str()); // redirect output to file

    std::ifstream file(file_name);
    result = { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

    return success;
}

vector<string> get_all_files_names_within_folder(string folder)
{
    vector<string> names;
    string search_path = folder + "/*";
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                string filename = fd.cFileName;
                names.push_back(fd.cFileName);
                logInfo("Found folder : %s", filename.c_str());
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return names;
}

void checkConnection() {

    bool isBoot = false;
    
    std::string cmd = "adb shell \"getprop | grep sys.boot_completed\"";
    std::string result;

    // always sleep for interval ms 
    const long interval = 500;

    while (!isBoot) {
        // detect device connection
        int ret = execute(cmd.c_str(), result);

        // device is booting
        if (result.empty())
        {
            logInfo("Device is booting");
            Sleep(interval);
        }

        // no device detected 
        if (ret != 0)
        {
            logWarning("No device detected");

            Sleep(interval);
        }
        else {
            logInfo("Device detected!!");
            isBoot = true;
        }

    }
    return;
}

void main(int argc, char** argv) {

    std::shared_ptr<std::thread> checkThread = std::make_shared<std::thread>(checkConnection);
    checkThread->join();

    std::string rootdir = "./";
    std::vector<std::string> foldernames;

    std::string cmd, result;
    std::string deviceID;

    
    //get device id
    cmd = "adb get-serialno";

    if (execute(cmd.c_str(), result)) {
        logError("Failed to read device ID");
        return;
    }
    else {
        deviceID = result.substr(0, result.size() - 1);
        logInfo("Devices id is %s", deviceID.c_str());
    }

    //create log folder
    system("mkdir log");


    logPath = rootdir + "log/" + deviceID + ".log";

    logInfo("=========================");

    foldernames = get_all_files_names_within_folder(rootdir);

    for (int x = 0; x < foldernames.size(); x++) {
        if (foldernames[x].find(deviceID) != std::string::npos) {
            logInfo("Found folder %s", foldernames[x].c_str());

            // grant root permission
            cmd = "adb root";
            if (execute(cmd.c_str(), result)) {
                logError("Fail to grant root permission");
                return;
            }
            else {
                logInfo("Grant root permission");
            }

            Sleep(500);

            // grant remount device
            cmd = "adb remount";
            if (execute(cmd.c_str(), result)) {
                logError("Fail to remount device");
                return;
            }
            else {
                logInfo("Remount device");
            }

            //push calibration files into device
            cmd = "adb push " + rootdir + foldernames[x] + "/CalibrationFiles /mnt/vendor/ ";
            if (execute(cmd.c_str(), result)) {
                logError("Fail to push calibration data, cmd : %s", cmd.c_str());
            }
            else {
                logInfo("Execute cmd : %s", cmd.c_str());
            }

            // adb sync
            cmd = "adb shell sync /mnt/vendor/CalibrationFiles/";
            if (execute(cmd.c_str(), result)) {
                logError("Fail to push execute command, cmd : %s", cmd.c_str());
            }
            else {
                logInfo("Execute cmd : %s", cmd.c_str());
            }

            //check all files exist

            for (int x = 0; x < FAC_CALIB_OUTPUT_FILES.size(); x++) {
                cmd = "adb shell \"cat " + FAC_CALIB_FLASH_TARGET_FOLDER + FAC_CALIB_OUTPUT_FILES[x] + "\"";
                if (execute(cmd.c_str(), result)) {
                    logError("%s not exist!!", FAC_CALIB_OUTPUT_FILES[x].c_str());
                    return;
                }
                else {
                    if (result.empty()) {
                        logError("%s is empty!!", FAC_CALIB_OUTPUT_FILES[x].c_str());
                        return;
                    }
                    logInfo("Check %s successed", FAC_CALIB_OUTPUT_FILES[x].c_str());
                }
            }
            logInfo("Restore successed !!!!");
            return;
        }
    }
    logError("DeviceID %s can't fnd match folder !!", deviceID.c_str());
}