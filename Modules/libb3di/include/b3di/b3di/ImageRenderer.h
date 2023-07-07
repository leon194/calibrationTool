#pragma once

#include <opencv2/core.hpp>
#include "CameraParams.h"

namespace b3di {

// A 2.5D image-based renderer that uses a depth image for shading computation
class ImageRenderer
{
public:
    
    // Phong shading params
    struct PhongParams {

        cv::Scalar ambientColor;
        cv::Scalar diffuseColor;
        cv::Scalar specularColor;
        double ka;
        double kd;
        double ks;
        double shininess;

        PhongParams() {
            ambientColor = diffuseColor = specularColor = cv::Scalar::all(255);
            ka = 0.5;
            kd = 0.8;
            ks = 1.0;
            shininess = 4.0;
        }

    };

    // For rendering teeth
    struct TeethShadingParams {
        cv::Scalar highlightColor;
        cv::Scalar toothColor;
        cv::Scalar shadowColor;
        double shininess;
        double brightnessScale;

        TeethShadingParams() {
            // some default values
            highlightColor = cv::Scalar(255, 255, 255);
            toothColor = cv::Scalar(223, 227, 222);
            shadowColor = cv::Scalar(113, 133, 151);
            shininess = 6.0;
            brightnessScale = 1.2;
        }
    };
    
    // Background (zero depth) pixel fill options
    enum BackgroundFill {
        NO_FILL
    };

    // Construct a rendering target by projecting 3d points to an image using imageCam
    // minZ and maxZ are the range of the z coordinate values of the 3d points
    // If targetRect is set, only the pixels inside the rect will be rendered and returned
    // Use densityScale to indicate the average projected point spacing in image pixes
    // If densityScale is larger than 1.0, the projected points are expected to create gaps in the image
    // and the renderer needs to attempt to fill them.
    // Currently, densityScale is ignored and the points are assumed to not create large gaps
    // Small holes (1 or 2 pixels) will be automatically filled.
    ImageRenderer(const std::vector<cv::Point3f>& points, const b3di::CameraParams& imageCam, 
        double minZ, double maxZ, const cv::Rect& targetRect = cv::Rect(), double densityScale=1.0,
        BackgroundFill bgFill=NO_FILL);    
    
    // Construct a rendering target by projecting 3d file to an image
    ImageRenderer(const cv::String& path, const b3di::CameraParams& imageCam, 
        const cv::Point3f& translation = cv::Point3f(), const cv::Vec3f& rotation = cv::Vec3f(),
        double minZ = 10, double maxZ = 1000, const cv::Rect& targetRect = cv::Rect(), 
        double densityScale=1.0, BackgroundFill bgFill=NO_FILL);

    virtual ~ImageRenderer();

    // Return the depth map used for shading computation
    // The depth map is an inverse of the distance map. The closest point has the largest value.
    // The depth map type is 16-bit unsigned ints and the size is defined by imageCam image size, 
    // or the targetRect in the constructor
    virtual cv::Mat getDepthMap() const;

    // Get an 8-bit alpha map where the closest point has the great value
    // and zero for background pixels
    virtual cv::Mat getAlphaMap() const;

    // Performs rendering with shadingParams using the Blinn-Phong reflection model
    // and return an RGB shaded image
    // lightDirection is a vector pointing at the direction of the light relative to the image plane. 
    // The default is a vector perpendicular to the image
    // Currently only (0, 0, 1) is supported (others are ignored for now)
    virtual bool renderPhongImage(const PhongParams& phongParams,
        cv::Mat& outputImage, 
        const cv::Vec3d& lightDirection=cv::Vec3d(0, 0, 1)) const;

    // Compute a diffuse intensity map: compute dot_product beween tagent normal vector and (0,0,1) vector
    // output value= staturate_cast<uchar>(dot_product * alpha + beta)
    // return an 8-bit gray scale image
    virtual bool renderDiffuseMap(cv::Mat& diffuseMap, double alpha=255.0, double beta=0.0) const;

    // Render as an 8-bit grapy scale map showing gradient differences
    virtual bool renderGradientMap(cv::Mat& gradientMap, double alpha=127.0/USHRT_MAX, double beta=0) const;

    // Compute a specular intensity map: compute dot_product beween tagent normal vector and (0,0,1) vector
    // output value= staturate_cast<uchar>(pow(dot_product, shininess)*alpha)
    // return an 8-bit gray scale image
    virtual bool renderSpecularMap(cv::Mat& specularMap, double shininess, double alpha=255.0) const;

    // experimental
    virtual bool renderTeethImage(cv::Mat& teethImage, const TeethShadingParams& params) const;
protected: 

    template <typename T>
    static bool renderDiffuse(const cv::Mat& grad_x, const cv::Mat& grad_y,
        cv::Mat& diffuseMap, const cv::Mat& srcMask = cv::Mat(), double alpha = 255, double beta = 0);

    template <typename T>
    static bool renderSpecular(const cv::Mat& grad_x, const cv::Mat& grad_y,
        cv::Mat& specularMap, const cv::Mat& srcMask = cv::Mat(), 
        double shininess=3.0, double alpha=255);

    template <typename T>
    static bool renderPhong(const cv::Mat& grad_x, const cv::Mat& grad_y,
        cv::Mat& shadedMap,
        cv::Scalar ambientColor, cv::Scalar diffuseColor, cv::Scalar specularColor,
        double ka, double kd, double ks, double shininess,
        const cv::Mat& srcMask = cv::Mat());

private:

    cv::Mat _depthMap;
    cv::Mat _alphaMap;
    cv::Mat _grad_x, _grad_y;

};

} // namespace b3di

