#pragma once

#ifdef ANDROID
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

struct android_backtrace_state
{
    void **current;
    void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, void* arg);
void signalHandler(int signum);
#endif