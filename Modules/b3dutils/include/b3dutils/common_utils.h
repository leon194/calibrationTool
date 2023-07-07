/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

#include <string>
#include <sstream>
#include <iomanip>  // std::setprecision

#include "B3DUtilsExportDef.h"

namespace b3dutils {

    // Return the current time in millisecond
    double DLLEXPORT now_ms(void);

    // Pause thread for N ms
    void DLLEXPORT sleepFor(unsigned milliseconds);

    // Convert number to string
    template <typename T>
    std::string DLLEXPORT to_string(T value, int precision = 0) {
        std::ostringstream os;
        
        if (precision > 0) {
            os << std::fixed << std::setprecision(precision) << value;
        }
        else {
            os << value;
        }

        return os.str();
    }

    std::string DLLEXPORT getPackageName();
} // namespace b3dutils
