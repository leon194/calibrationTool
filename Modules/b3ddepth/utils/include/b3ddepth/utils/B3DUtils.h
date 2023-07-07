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

#include <b3ddepth/core/B3DImage.h>

namespace b3dd {

    // Return the current time in millisecond
    double DLLEXPORT now_ms(void);

    // Pause thread for N ms
    void DLLEXPORT sleepFor(unsigned milliseconds);

    // Convert number to string
    template <typename T> DLLEXPORT
    std::string to_string(T value, int precision = 0) {
        std::ostringstream os;
        
        if (precision > 0) {
            os << std::fixed << std::setprecision(precision) << value;
        }
        else {
            os << value;
        }

        return os.str();
    }

} // namespace b3dd
