#pragma once

// Shouldn't need this since we have #pragma once
//#ifndef PLATFORM_H
//#define PLATFORM_H

// Add platform specific includes here
// "WIN32" is defined by Windows SDK,  "_WIN32" is defined by compiler for Windows
#ifdef WIN32
  // Including SDKDDKVer.h defines the highest available Windows platform.
  // If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
  // set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
  #include <SDKDDKVer.h>

  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
  #endif

  // Windows Header Files

  // remove ancient macros, otherwise you get warning:
  // not enough actual parameters for macro 'max'
  #ifndef NOMINMAX
    #define NOMINMAX
  #endif

  #include <windows.h>
  #include <Shellapi.h>

  // C RunTime Header Files
  #include <stdio.h>
  #include <tchar.h>

  //#include <stdio.h>
  //#include <stdlib.h>
  //#include <malloc.h>
  //#include <memory.h>
  //#include <crtdbg.h>

  #ifdef _UNICODE
    #if defined _M_IX86
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #elif defined _M_X64
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #else
      #pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
    #endif
  #endif  // _UNICODE

#endif // WIN32

// If Apple device
#ifdef __APPLE__
  #include <TargetConditionals.h>  // TARGET_OS_IPHONE macro is defined in this file

  #ifdef TARGET_OS_IPHONE
    #include <unistd.h>  // For the mySleep() in B3d_utils, and rmdir() in FileUtils=
  #endif
#endif

#ifdef ANDROID
  #include <android/log.h>
  #include <unistd.h>  // For the mySleep() in B3d_utils, and rmdir() in FileUtils=
  #define DISABLE_GLB_EXPORT  // currently we cannot export GLB on Android platform
#endif

// USHRT_MAX does not exist on ios
#ifndef USHRT_MAX
  #define USHRT_MAX 0xffff
#endif

// End of platform.h file
