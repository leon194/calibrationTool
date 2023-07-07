#include "common_utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <time.h>


// #include "platform.h"
#ifdef ANDROID
  #include <android/log.h>
  #include <unistd.h>  // For the mySleep() in B3d_utils, and rmdir() in FileUtils=
#endif

#include <chrono>
#include <thread>



namespace b3dutils {

void sleepFor(unsigned milliseconds) {
    std::chrono::duration<int, std::milli> timespan(milliseconds);
    std::this_thread::sleep_for(timespan);
}

double now_ms(void) {
#ifdef WIN32
    //#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)
    /* return current time in milliseconds */
    //	clock_t timeCur = clock() / CLOCKS_PER_MS;
    clock_t timeCur = clock();
    return (double)timeCur;
#elif defined(ANDROID) || defined(TARGET_OS_IPHONE)
    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double)res.tv_nsec / 1e6;
#else
    // NOT defined
    return 0.0;
#endif
}

    std::string getPackageName() {
#ifdef __ANDROID__
        pid_t pid = getpid();

        char path[64] = { 0 };
        sprintf(path, "/proc/%d/cmdline", pid);
        FILE *cmdline = fopen(path, "r");
        char application_id[64] = { 0 };
        if (cmdline) {
            fread(application_id, sizeof(application_id), 1, cmdline);
            fclose(cmdline);
        }

        return std::string(application_id);
#else
        return nullptr;
#endif
    }

}
