
#pragma once


#include <vector>
#include <stdio.h>

#include "B3d_defs.h"
#include "CameraParams.h"
#include "XForm.h"
#include "TriMesh.h"
#include "PolyMesh.h"
#include "B3d_utils.h"
#include "DepthUnitConverter.h"


namespace b3di {

//typedef std::vector<cv::Point3f> PointCloud3D;

class _PointCloudData;

// A 3d bounding box
// x, y, z are coordindates of a corner
struct BoundBox {
    float x, y, z;
    float width, height, depth;
    BoundBox() { x = y = z = width = height = depth = 0.0; }
    BoundBox(float _x, float _y, float _z, float _width, float _height, float _depth) {
        x = _x, y = _y, z = _z, width = _width, height = _height, depth = _depth;
    }

    bool empty() {
        return (width <= 0 || height <= 0 || depth <= 0);
    }

    void dump() {
        std::cout << "x=" << x << " y=" << y << " z=" << z << " w=" << width << " h=" << height << " d=" << depth << std::endl;
    }
};

// Tilt the ouptut head by this angle (X rotation degrees) 
static const float OUTPUT_TILT_ANGLE = 7.0f;

class PointCloud {
public:

	PointCloud();
	virtual ~PointCloud();

	PointCloud(const PointCloud& a);

//	PointCloud(int grid_width, int grid_height);

    // Construct a point cloud from a list of 3d points. If grid_width and grid_height are > 0, the points form a 2d grid
    // Number of items in points3d should match grid_width*grid_height (if they are not zero)
	PointCloud(const std::vector<cv::Point3f>& points3d, int grid_width=0, int grid_height=0) { 
        create(points3d, grid_width, grid_height); 
    }

    //// Construct a point cloud from a list of grid points. 
    //// Number of items in points3d should match gridSize.width*gridSize.height
    //// Texture size specifies the texture image size and may not be the same as gridSize
    //PointCloud(const std::vector<cv::Point3f>& points3d, const cv::Size& gridSize, const cv::Size& textureSize) {
    //    create(points3d, gridSize, textureSize);
    //}

	// Construct a point cloud from a cylindrical map with params defined in cylCam
	PointCloud(const cv::Mat& cylDepthMap, const b3di::CameraParams& cylCam)
	{
		create(cylDepthMap, cylCam);
	}

	// Clear point cloud
    virtual void clear();

    virtual bool empty() const {
		return (_points.size() == 0);
	};

    virtual PointCloud& operator= (const PointCloud& a) {
		_points = a._points;
		_texCoords = a._texCoords;
		//_imageSize = a._imageSize;
        _gridSize=a._gridSize;
        _gridOffset = a._gridOffset;
		//_isRangeGrid = a._isRangeGrid;
		//_meshRect = a._meshRect;
		// Invalidate the current trimesh data; They will be recomputed when needed
		clearTriMesh();
		return *this;
	}

    // Construct a point cloud from a list of 3d points. If grid_width and grid_height are > 0, the points form a 2d grid
    // Number of items in points3d should match grid_width*grid_height (if they are not zero)
    // Texture size is assumed to be the same as grid size
    virtual void create(const std::vector<cv::Point3f>& points3d, int grid_width=0, int grid_height=0);

    //// Construct a point cloud from a list of grid points. 
    //// Number of items in points3d should match gridSize.width*gridSize.height
    //// Texture size specifies the texture image size and may not be the same as gridSize
    //virtual void create(const std::vector<cv::Point3f>& points3d, const cv::Size& gridSize, const cv::Size& textureSize);

    // Construct a point cloud from a depth map
    // if depthRect is not Rect(), it defines a sub rect in depthMap to construct from
    // minDepth/maxDepth defines the depth cropping range
    // depthRes is ignored
    virtual void create(const cv::Mat& depthMap, const cv::Mat& intrinsic,
		ushort minDepth = (ushort)g_depthUnitConverter.minDepth(),
		ushort maxDepth = (ushort)g_depthUnitConverter.maxDepth(),
		float depthRes = 0,
		const cv::Rect& depthRect = cv::Rect());

	//void create(const cv::Mat& depthMap, const cv::Mat& intrinsic,
	//	double minZ, double maxZ,
	//	const cv::Rect& depthRect = cv::Rect());

    // Construct a point cloud from a cylindrical depth map (created from buildMesh)
    virtual void create(const cv::Mat& cylDepthMap, const b3di::CameraParams& cylCam, 
        const cv::Rect& cropRect=cv::Rect(), const std::vector<cv::Point>& contourPoints=std::vector<cv::Point>());

	// Read yml, png, ply, obj, stl files. 
    // yml file should contain a "Point3f" vector of points
	// For reading png (depth), cam is required
	// minZ and maxZ are the range of z values for depth image points to keep (0 for no limit). only applicable for reading png file
	// bounds is the rect in the depth image for points to keep
	virtual bool readFile(const cv::String& inputFile, const b3di::CameraParams& cam = b3di::CameraParams());
	virtual bool writeFile(const cv::String& outputFile, bool writeTextureCoords=false);

	// If you alter the points, you must call clearTriMesh to invalidate the trimesh data
    virtual std::vector<cv::Point3f>& points() { return _points; }

    virtual void setPoints(const std::vector<cv::Point3f>& points3d) { _points = points3d; }
    virtual const std::vector<cv::Point3f>& getPoints() const { return _points; }

    // return a downsampled set of points.
    // downloadSampleRate should be >=1. For example, 2 means return 1/2 of points
    virtual void downsamplePoints(std::vector<cv::Point3f>& points, int downsampleRate) const;

    // static version of upsamplePoints
	// if nearest is true, upsampling with nearest point instead of interpolation
    static void upsamplePoints(const std::vector<cv::Point3f>& srcPoints, int gridWidth, int gridHeight, std::vector<cv::Point3f>& dstPoints,
		bool nearest=false);

    // Return an upsampled set of points by 2x in each direction
    // This only works if the points form a grid
    // The returned number will be (2w-1)*(2h-1), where w and h are width and height of the point grid.
    virtual void upsamplePoints(std::vector<cv::Point3f>& points) const {
        return upsamplePoints(_points, _gridSize.width, _gridSize.height, points);
    }

	// Only applicable if the point cloud forms a mesh
    virtual cv::Size getMeshSize() const { return _gridSize; }
    virtual void setMeshSize(cv::Size s) { _gridSize = s; }

    virtual cv::Rect getMeshRect() const { 
        return cv::Rect(_gridOffset.x, _gridOffset.y, _gridSize.width, _gridSize.height);
        //return _meshRect; 
    }

	// Transform the points by a 4x4 xf
	virtual void transformPoints(const trimesh::xform& xf);

	// Project the 3d points to 2d image space using camera intrinsic params
	// If imageMaskSize is not empty, also return a mask of the projected points in imageMaskSize
	// if invalidPointIndices is not null and imageMaskSize is set, 
	// return a vector of indices of points outside of the mask image when projected
	// colorFrame is optional and for debugging. It shouls match imageMaskSize
	virtual void projectPoints(const cv::Mat& intrinsic, std::vector<cv::Point2f>& projectedPoints, 
		cv::Mat& projectedPointsMask, cv::Size imageMaskSize=cv::Size(), 
		std::vector<int>* invalidPointIndices =NULL, const cv::Mat& colorFrame=cv::Mat()
		) const;

	// Project the 3d points to a depth image
	virtual void projectPointsToDepth(const cv::Mat& intrinsic, const cv::Mat& distCoeffs, cv::Size depthImgSize, cv::Mat& depthMap, 
		double src2DstScale = 1.0, bool fillHoles = true, double depthResolution = 0) const;

    // Project the 3d points to a 32-bit depth image
    virtual void projectPointsToDepth32(const cv::Mat& intrinsic, cv::Size depthImgSize, cv::Mat& depthMap,
        bool fillHoles = true) const;

	// static version of projectPointsToDepth (can be called without a PointCloud object)
	static void projectPointsToDepth(const std::vector<cv::Point3f>& points3d, const cv::Mat& intrinsic, const cv::Mat& distCoeffs, cv::Size depthImgSize, cv::Mat& depthMap, 
		double src2DstScale = 1.0, bool fillHoles = false, double depthResolution = 0);

    // Similar to projectPointsToDepth above but create a 32-bit floating point depth map
    // depthImageSize should match the point density to not create large holes (pixel-sized holes will be filled in fillHoles is true)
    static void projectPointsToDepth32(const std::vector<cv::Point3f>& points3d, const cv::Mat& intrinsic, cv::Size depthImgSize, cv::Mat& depthMap,
        bool fillHoles = false);

    // project 3d points to a depth image using camera intrinsic.
    // The z value is converted to depth using depthVal = (zVal - zOffset)*zScale
    // outSize is the output depth map size
    static void projectPointsToDepth(const std::vector<cv::Point3f>& points3d, const cv::Mat& intrinsic,
        float zOffset, float zScale, cv::Size outSize, cv::Mat& depthMap);

	// Project 3d points with a texture map to dstImage
	// The point cloud must be a mesh
	// Forward projection to texture map doesn't work well because it will create many holes
	// use projectPoints to create an inverse map instead
	virtual void projectToImage(const cv::Mat& textureMap, const cv::Mat& intrinsic, 
		cv::Size dstSize, cv::Mat& dstImage,
		double src2DstScale = 1.0, bool fillHoles = true, double depthResolution = 0) const;

	// static version of projectToImage (can be called without a PointCloud object)
	// The point cloud must be a mesh
	static void projectToImage(const std::vector<cv::Point3f>& points3d, cv::Rect meshRect,
		const cv::Mat& textureMap, const cv::Mat& intrinsic, cv::Size dstSize, cv::Mat& dstImage,
		double src2DstScale = 1.0, bool fillHoles = true, double depthResolution = 0);


	// Get the transformation form the world space to the head space
	// cylCam is the cylindrical depth camera that defines the head space
	// coordScale is used to scale the output coordinates
	static trimesh::xform getHeadSpaceTransform(const b3di::CameraParams& cylCam, double coordScale=1.0);

    // Find the bounding box of points
    // if boundBox is not empty, including only points inside the boundBox
    static BoundBox getBoundBox(const std::vector<cv::Point3f>& points, const BoundBox& boundBox = BoundBox());
    static cv::Point3f getCentroid(const std::vector<cv::Point3f>& points, const BoundBox& boundBox = BoundBox());
    // Return the number of points inside a bounding box
    static int countPoints(const std::vector<cv::Point3f>& points, const b3di::BoundBox& bbox);
    // Return points inside a bounding box
    static void getPointsInsideBox(const std::vector<cv::Point3f>& points, const b3di::BoundBox& bbox, std::vector<cv::Point3f>& insidePoints);

    virtual BoundBox getBoundBox(const b3di::BoundBox& boundBox = BoundBox()) const { return getBoundBox(_points, boundBox); }
    virtual cv::Point3f getCentroid(const b3di::BoundBox& boundBox) const { return getCentroid(_points, boundBox); }
    virtual int countPoints(const b3di::BoundBox& boundBox) const { return countPoints(_points, boundBox); }
    virtual void getPointsInsideBox(const b3di::BoundBox& boundBox, std::vector<cv::Point3f>& insidePoints) const {
        return getPointsInsideBox(_points, boundBox, insidePoints); }

	//static void projectPointsToXYMap(const std::vector<cv::Point3f>& points3d, const cv::Mat& intrinsic, const cv::Mat& distCoeffs, cv::Size xyMapSize, cv::Mat& xyMap,
	//	double imageScale = 1.0, bool fillHoles = false);


//	virtual void projectPointsToCylindricalDepth(const Mat& intrinsic, const Point3f& projectionCenter, Size cylImgSize, Mat& cylDepthMap) const;
	//virtual void projectPointsToCylindricalDepth(const b3di::CameraParams& cylCam, cv::Mat& cylDepthMap, 
	//	const cv::Mat& maskImage = cv::Mat(), const trimesh::xform& pcloud_xf = trimesh::xform()) const;
	virtual void projectPointsToCylindricalDepth(const b3di::CameraParams& cylCam, cv::Mat& cylDepthMap,
		const cv::Rect& cropRect = cv::Rect(), const trimesh::xform& pcloud_xf = trimesh::xform(), bool fill4=true) const;

	virtual void projectPointsToCylindricalDepth32(const b3di::CameraParams& cylCam, cv::Mat& cylDepthMap,
		const cv::Rect& cropRect = cv::Rect(), const trimesh::xform& pcloud_xf = trimesh::xform(), bool fill4 = true) const;
	virtual void projectPointsToCylindricalDepth32_span(const b3di::CameraParams& cylCam, cv::Mat& cylDepthMap,
		const cv::Rect& cropRect = cv::Rect(), const trimesh::xform& pcloud_xf = trimesh::xform()) const;

	// Reproject points to the cylindrical space and return projected points projectedPts
	virtual void projectPointsToCylindricalSpace(const b3di::CameraParams& cylCam, std::vector<cv::Point3f>& projectedPts,
		const trimesh::xform& pcloud_xf = trimesh::xform()) const;

    virtual cv::Point3f getCentroid() const;

	// Converts points to a PolyMesh
	// vertexSize can 3 (XYZ), 6 (XYZ, normal), 8 (XYZ, normal, texture corrds)
	// edgeNum can be 3 (triangles) or 4 (quads)
	// If simplifyRatio is not zero, the mesh will be simplifed by the ratio (<1.0)
	// If simplifyRatio is zero and targetNumVertices is not zero, the mesh will be simplified to have targetNumVertices vertices
	// cam is required for computing texture coordinates of the simplified mesh
	// If depthImage and cam are provided, normal vectors are computed from depthImage
	// If edgeLenThresh is not 0, use it to eclude very elongated faces whose edge length exceeds the threshold
	// If wrapMesh is true, the mesh will wrap around horizontally
	// If watertight is true, the output mesh will be watertight. This is currently only supported if wrapMesh is true
    // Set pointsXf to apply a transformation to the points
    // Set leftHandedCoordSpace to true for left-handed coordinate system
	virtual bool pointsToPolyMesh(b3di::PolyMesh& polyMesh, int vertexSize = 3, int edgeNum = 3, double simplifyRatio = 0, int targetNumVertices=0,
		const b3di::CameraParams& cam = b3di::CameraParams(), double coordScale=1.0, double edgeLenThresh=0.0, 
		bool wrapMesh=false, bool watertight =false, 
        trimesh::xform pointsXf= trimesh::xform(), bool leftHandedSpace =false
		) const;

	// Register this PC to targetPC
	// srcXf and targetXf are the initial transformation for this PC and target PC respectively
	// Return srcXf and regisErr
	// srcCenter and targetCenter are the centroid points of the two registered PCs
    // icpSpeed is the computation speed. Valid value should be 0, 1, 2
    // 0 is the fastest (but less accurate), 2 is the slowest (but more accurate). 1 seems to work well in most cases
    // call prepareForICP prior to registerICP if there are any changes to the points
    virtual bool prepareForICP();
	virtual bool registerICP(const PointCloud& targetPC, const trimesh::xform& targetXf, trimesh::xform& srcXf, cv::Point3f& srcCenter, 
		cv::Point3f& targetCenter, float& registErr, int icpSpeed=1);
	
	// Convert a depth image to 3d points
	// if sampleFactor is great than 1, the depthMap will be sub sampled by the factor
	// depthRect defines a sub region to convert if it is set
	virtual void depthToPoints(const cv::Mat& depthMap, const cv::Mat& intrinsic,
		ushort minDepth = (ushort)g_depthUnitConverter.minDepth(),
		ushort maxDepth = (ushort)g_depthUnitConverter.maxDepth(),
		float depthRes = 0,
		cv::Rect depthRect = cv::Rect());

	static void depthToXyz(const cv::Mat& depthMap, const cv::Mat& cameraMatrix, cv::Mat& xyzMap, ushort minDepth = 0, ushort maxDepth = USHRT_MAX, cv::Rect depthRect = cv::Rect());
	static void rotateXyzMap(const cv::Mat& rotMtx, cv::Mat& xyzMap, double zScale=1.0);

	static void cropPoints(const std::vector<cv::Point3f>& srcPoints, std::vector<cv::Point3f>& dstPoints, float nearZ, float farZ);

	// Compute a normal map from a point map (each pixel is a Point3f)
	static void computeNormalMap(const cv::Mat& ptsMap, cv::Mat& normalMap);

    // Render the points to create an RGB shaded image using imageCam params
    // The points are assumed to be dense enough to not create large holes (small holes <3 pixels will be filled through filtering)
    // renderParams are shading params for Blinn-Phong model
    // minZ and maxZ are the z-cropping range
    // targetImage is the rendered image
    // If targetRect is not empty, render only the targetRect region of the imageCam image 
    //static void render(const std::vector<cv::Point3f>& points, const b3di::CameraParams& imageCam,
    //    const RenderParams& renderParams,
    //    float minZ, float maxZ, 
    //    cv::Mat& targetImage, cv::Mat& targetMask, 
    //    cv::Rect targetRect = cv::Rect());

protected:

    // Get TriMesh; Convert from points to trimesh if necessary
    // Set getTextureCoords to true to generate texture coordinates
    cv::Ptr<trimesh::TriMesh> getTriMesh(bool getTextureCoords = false, bool getNormals = false);
    // Clear the current trimesh data
    virtual void clearTriMesh();

    // Converts points to trimesh and vice versa
    virtual void pointsToTriMesh(bool getTextureCoords = false, bool getNormals = false);
    virtual void trimeshToPoints();

    bool readAsciiStl(const cv::String& fileName);


private:


	std::vector<cv::Point3f> _points;		// 3d points
	std::vector<cv::Point2f> _texCoords;		// texture coordinates
    cv::Size _gridSize;     // number of points in horizontal and vertical directions if the points form a 2d grid
    cv::Point _gridOffset;  // texture map coordinate offset

	//cv::Size _imageSize;		// Only used if the points form a 2d grid. Set image size for the points (used for computing texture coordinates)
    //cv::Rect _meshRect;     // a rect in the texture image the points map to (if they form a grid)

    //bool _isRangeGrid;		// the points are from a grid


	cv::Ptr<_PointCloudData> _data;

};

} // namespace b3di
