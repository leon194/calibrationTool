
#pragma once

typedef bool(*ProgressFuncPtr)(int stepCount, int totalSteps, void* userData);

struct ProgressTracker {
	static int stepCount;
	static int totalSteps;
	static void* userData;
	static ProgressFuncPtr progressFuncPtr;
	static bool updateProgress() {
#pragma omp critical
		{
			ProgressTracker::stepCount++;
			if (ProgressTracker::stepCount > ProgressTracker::totalSteps)
				ProgressTracker::stepCount = ProgressTracker::totalSteps;
		}
		if (ProgressTracker::progressFuncPtr)
			return (*ProgressTracker::progressFuncPtr)(ProgressTracker::stepCount, ProgressTracker::totalSteps, ProgressTracker::userData);
		else
			return true;

	}
};

