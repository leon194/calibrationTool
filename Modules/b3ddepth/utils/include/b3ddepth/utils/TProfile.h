/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once

#include <time.h>

#include <b3ddepth/utils/B3DUtils.h>
#include <b3ddepth/utils/TLog.h>


namespace b3dd {

class DLLEXPORT TProfile
{
public:

    TProfile(){
		reset();
    };

	//start the clock or resume
    void start(bool resetCounter=false) {
        timerStart = now_ms();
		if (resetCounter)
			counter = 0;
    };
    //pause the clock or stop
    void pause() {
        timeTotal += (now_ms()-timerStart);
		counter++;
    };

	// get the number of times the timer paused
	int getPauseCounter() const { return counter; }

    // get the total time counted
	// return in milli-sec, unless timeInSec is true (return in sec)
    double getTotalTime(bool timeInSec=false) const {
		if (timeInSec)
			return timeTotal / 1000.0;
		else
			return timeTotal;
    };

	//get the average time for each start/pause cycle
	double getAverageTime() const {
		if (counter)
			return timeTotal / counter;
		else
			return 0;
	};

    // FIXME: if the calling sequence is:
    // start() -> reset() -> pause() -> start() -> pause() ...
    // the result will be messed up
	void reset() {
		timerStart = timeTotal = 0;
		counter = 0;
	}

protected:
	double timeTotal;
	double timerStart;
	int counter;
};

} // namespace b3dd
