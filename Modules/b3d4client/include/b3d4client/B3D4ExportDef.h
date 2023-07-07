/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once


#ifndef DLLEXPORT
    #ifdef WIN32
        #define DLLEXPORT __declspec( dllexport )
    #else
        #define DLLEXPORT
    #endif
#endif
