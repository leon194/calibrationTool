#pragma once


#include "FaceModelTemplate.h"

namespace b3di {

const cv::String FaceModelTemplateHB_Type="hbfrontal";

class FaceModelTemplateHB : public FaceModelTemplate {

public:

	FaceModelTemplateHB();
	virtual ~FaceModelTemplateHB();

	// return a PolyMesh from the template data and its meshCam
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// if baseShape is true, return the mesh formed by landmarks triangles only
	// If interiorFacesOnly is true, return only mesh faces that belong to interior landmark triangles
	virtual void getMesh(const b3di::CameraParams& meshCam, b3di::PolyMesh& pm,
		bool baseShape =false) const;

	// compute fitted texture map from srcTexture
	virtual void computeFittedTextureMap(const cv::Mat& srcTexture, cv::Mat& fittedTexture) const;

protected:


	// Special function for HB template models only
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// morph an aligned HB poly mesh to this template by interpolating the vertices
	// modelTemplate, pmTemplate are files of the original template model
	// meshCam is the mesh cam file of this template model
	// alpha is a value between 0 and 1 that controls the morphing from pm to the template
	// alpha=0 will keep pm as is and alpha=1 will morph to the template completely
	// vertexAlpha, if not empty, controls the per vertex morph and will be multiplied with alpha
	// to detemine the morphing factor for each vertex.
	// pm vertices will be modified on return if alpha is not 0
	// outputScale controls the scaling of the output PolyMesh coordinates
	void morphHBMeshToTemplate(b3di::PolyMesh& pm,
		float alpha, const std::vector<float>& vertexAlpha, FitScale scaleType = FIT_AVERAGE) const;


	// A special computeAlpha function for HB templates only
	// If the template is not a complete template, originalTemplate must be set prior to calling this function
	// compute alpha as a function of its pixel distance to the border contour 
	// in the cylindrical space
	// blendDist is the blending region distance from the border 
	// blendDist should be a normalized (0-1) pixel distance in the cylindrical image space.
	// zeroDist the distance from the borader where the blending alpha goes to zero. If zeroDist is 0, the blending will go to 0 at the boundary.
	// If the template is a partial template (missing meshVerts), modelTemplate is the original template required to provide meshVerts
	// foreheadBlendingRatio is how much we are using the for
	void computeHBAlpha(cv::Mat& alphaMap,
		float blendDist = 0.03f, float zeroDist = 0.01f, float foreheadBlendingRatio = 0.5f, bool erodeMap = true) const;


};

} // namespace b3di
