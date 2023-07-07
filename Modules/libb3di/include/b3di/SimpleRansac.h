#ifndef SIMPLERANSAC_H
#define SIMPLERANSAC_H

// https://userpages.uni-koblenz.de/~fneuhaus/wp/code-snippets/

#include <array>
#include <random>
#include <set>
#include <vector>

// inlierCount is input/output
// If inlinerCount>0, returns immediately if the number of inliers found exceed inlierCount
// Otherwise, run numIterations times
template <typename Model_, typename DataType>
Model_ ransac(const std::vector<DataType>& data, double threshold, int& numIterations, int& inlierCount,
    unsigned int randomSeed = std::random_device{}())
{
    if (data.size() < Model_::ModelSize)
        throw std::runtime_error("Not enough data");

    std::mt19937 engine{ randomSeed };
    std::uniform_int_distribution<size_t> dis{ 0, data.size() - 1 };

    int bestInliers = -1;
    Model_ bestModel;
	int it = 0;
    for (; it < numIterations; it++)
    {
        // select points
        int found = 0;
        std::array<size_t, Model_::ModelSize> indices;
        std::set<size_t> usedIndices;
        while (found < Model_::ModelSize)
        {
            size_t sample = dis(engine);
            if (usedIndices.find(sample) != usedIndices.end())
                continue;
            usedIndices.insert(sample);
            indices[found++] = sample;
        }

        // compute model
        Model_ m;
        m.compute(data, indices);

        int inliers = m.computeInliers(data, threshold);
        if (inliers > bestInliers)
        {
            bestModel = m;
            bestInliers = inliers;
        }

		// break if we have exceeded the required number of inliers (inlierCount>0)
		if (inlierCount>0 && bestInliers>inlierCount) {
			it++;
			break;
		}
    }
	numIterations = it;
    bestModel.refine(data, threshold);
	inlierCount = bestInliers;
    return bestModel;
}

#endif
