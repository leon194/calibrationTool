# b3d4_clientapp

## Synopsis
B3D4 Client App for control camera devices
## Prerequisits.
### Operating systems.
* Android 8.1 (Oreo)
### Applications and drivers.
* [Android Studio 4.1.1](https://developer.android.com/studio) 
* [CMake 3.10.2](https://cmake.org/download/)
* [OpenCv 3.4.1](https://opencv.org/opencv-3-4-1/)
* [Cmake-Gui](https://cmake.org/download/)
## Building.
### Android
    1. download Android Studio 
    2. include gradle in AndroidClient\ClientApp
    3. update sdk build tool, cmake version via sdk manager
    4. start to build the project
### Windows TestApp
    1. install opencv via opencv website
    2. install cmake-gui tool and use CMakeLists.txt at root folder to build

## update client app.
    arc device didn't have display screen, so we config some default app permission for client app, we suggest use bellow way to update client app
    inorder to avoid some permission lost.
### required files
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\outputs\apk\debug\ArcClient-v{version for git tag}-debug.apk
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libb3d4client.so
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libb3d4clientJNI.so
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libb3ddepth.so
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libb3dutils.so
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libc++_shared.so
    b3d4_clientapp\AndroidClient\ClientApp\b3d4app\build\intermediates\transforms\mergeJniLibs\debug\0\lib\arm64-v8a\libopencv_java3.so
### use host app
    1. create a new tag and zip all the required file
    2. rename the zip folder use the same name as ArcClient-v{version for git tag}-debug.apk
       for example, if ArcClient-v2.0.7-debug.apk, then the zip file name should use 2.0.7.zip
    3. copy zip file to ARCHost\bin\clients\2.0.7.zip and restart host service
### use command line
    1. open a command line window or power shell
    2. cd to the directory to your ndk location
    3. use bellow command to update client app
       adb root
       adb remount
       adb push ArcClient-v2.0.7-debug.apk /system/priv-app/B3D4App/B3D4App.apk
       adb push libb3d4client.so /system/lib64/libb3d4client.so
       adb push libb3d4clientJNI.so /system/lib64/libb3d4clientJNI.so
       adb push libb3ddepth.so /system/lib64/libb3ddepth.so
       adb push libc++_shared.so /system/lib64/libc++_shared.so
       adb push libopencv_java3.so /system/lib64/libopencv_java3.so
       adb reboot
       
*Copyright (C) Bellus3D, Inc. All rights reserved.*
