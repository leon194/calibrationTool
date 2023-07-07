
#pragma once

#include "HeadMeshProcessor.h"


// Deprecated. Use HeadMeshBuilder::process() instead
// Return an error code of BUILDMESH_ERROR
int buildMesh(BuildMesh_Input& input, BuildMesh_Output& output) {

	b3di::HeadMeshProcessor hp;
	return (int)hp.process(input, output);
}

