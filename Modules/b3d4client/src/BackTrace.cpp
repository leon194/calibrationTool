//
// Created by LeonFeng on 2021/5/18.
//
#ifdef __ANDROID__
#include "backTrace.h"
#include <b3ddepth/utils/TLog.h>

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context,
                                            void* arg)
{
    android_backtrace_state* state = (android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

void signalHandler(int signum)
{
    logStackTrace("caught signal number %d",signum);
    logStackTrace("== android stack dump ==");

    const int max = 100;
    void* buffer[max];

    android_backtrace_state state;
    state.current = buffer;
    state.end = buffer + max;

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    for (int idx = 0; idx < count; idx++)
    {
        const void* addr = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname)
        {
            symbol = info.dli_sname;
        }
        int status = 0;
        char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

        if(demangled != nullptr) {
            logStackTrace("%03d: %p %s %s",
                          idx,
                          addr,
                          demangled,
                          info.dli_fname);
            free(demangled);
        }
    }

    logStackTrace("== android stack dump done ==");
}
#endif