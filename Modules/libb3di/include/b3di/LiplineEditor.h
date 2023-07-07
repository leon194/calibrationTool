#pragma once

#include <opencv2/core.hpp>
#include "HeadMeshI.h"
#include "FaceTracker.h"

namespace b3di {

// HeadMesh lipline interactive editor
class LiplineEditor
{
public:

    LiplineEditor();
    virtual ~LiplineEditor() {};

    // The head model whose lipline is to be edited
    // mouthPict is an image of the mouth area to trace the lipline with
    //virtual bool newSession(const b3di::HeadMeshI& headModel, cv::Mat& mouthPict);
    virtual bool newSession(b3di::HeadMeshI* headModel, cv::Mat& mouthPict);

    // Return the current control points for the lipline
    // The coordinates of the points are in the mouthPict space
    virtual std::vector<cv::Point> getControlPoints() const;

    // Move a control point in the control points vector
    // pointIndex is an index of the point to modify
    // moveTo is the new position of the point in the mouthPict space
    virtual bool moveControlPoint(int pointIndex, cv::Point moveTo);
   
    // Return a vector of points created from the current control points
    // The coordinates of the points are in the mouthPict space
    virtual std::vector<cv::Point> getLipline() const;

    // Return a HeadMesh feature mask, which can be used as a paramter to
    // HeadMeshI::getMesh function to cut out the teeth area based on
    // the current lipline
    virtual cv::Mat getFeatureMask() const;

    // Reset control points to those in the input headModel
    virtual void resetControlPoints();

    // Save the edited lipline to the target headModel
    //virtual void applyToModel(HeadMeshI& headModel) const;
    virtual bool applyToModel() const;

private:
    cv::Rect _mouthRect;
    cv::Size _maskImageSize;
    std::vector<cv::Point> _controlPoints;
    FaceTracker::FaceLandmarks _faceLandmarks;
    b3di::HeadMeshI* _headMeshPtr;
};

} // namespace b3di

