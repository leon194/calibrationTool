#pragma once

#include <vector>
#include "opencv2/core.hpp"
#include "XForm.h"

namespace b3di {

// PolyMesh coordinates unit (1 means 1mm and 10 means 1 cm)
// Set this to 10.0 so the output OBJ file is in cm, which is the standard
const double POLYMESH_COORD_UNIT = 10.0;		// in mm

// Number of lower bits (out of 32) for vertex index
// Higher bits are used for texture only vertices
const int VERTEX_NUM_BITS = 20;		

/// <summary>
/// A Polygonal Mesh class for triangle or quad meshes. Coordinates can be float or double
/// </summary>
template <class T>
class PolyMesh_ {
public:

	typedef int VertexIndex;

	struct Vertex_XYZ {
		T x, y, z;
	};
	struct Vertex_XYZN {
		T x, y, z, nx, ny, nz;
	};
	struct Vertex_XYZNT {
		T x, y, z, nx, ny, nz, tx, ty;
	};

	// Vertext data offsets to the vertices array
	enum VertexOffset {
		VERTEX_OFFSET_XYZ = 0,
		VERTEX_OFFSET_NORMAL = 3,
		VERTEX_OFFSET_UV = 6
	};

	enum VertexSize {
		VERTEX_SIZE_XYZ = 3,
		VERTEX_SIZE_XYZN = 6,
		VERTEX_SIZE_XYZNT = 8
	};

	enum EdgeNum {
		EDGE_NUM_TRI=3,
		EDGE_NUM_QUAD=4
	};

	enum VertexFlag {
		VERTEX_HAS_NORMAL = 2,
		VERTEX_HAS_UV=4
	};

	enum VertexType {
		VERTEX_XYZ,
		VERTEX_UV,
		VERTEX_NORMAL,
		VERTEX_ALL
	};

    // This should be the same as the one in PolyMesh
    // copied here so we don't need to depend on PolyMesh.h
    enum CoordSpace {
        CS_UNKNOWN,                 // unknown coordiate space 
        CS_RIGHT_Y_UP,              // right-handed coordinate system where Y axis points up  (default)
        CS_RIGHT_Z_UP,              // right-handed coordinate system where Z axis points up
        CS_LEFT_Y_UP,               // left-handed coordinate system where Y axis points up
        CS_LEFT_Z_UP                // left-handed coordinate system where Z axis points up
    };

	PolyMesh_(int numberOfVertices = 0, int numberOfFaces = 0, VertexSize sizePerVertex = VERTEX_SIZE_XYZNT, EdgeNum edgesPerFace = EDGE_NUM_TRI,
		unsigned char flag = VERTEX_HAS_NORMAL | VERTEX_HAS_UV) 
	{
		create(numberOfVertices, numberOfFaces, sizePerVertex, edgesPerFace);
	}

	// Create a PolyMesh from another one. If vertexSize is 0, the mesh will have the same vertex size as srcMesh
	// Otherwise, create one with the vertexSize
	PolyMesh_(const PolyMesh_<T>& srcMesh, int vertexSize=0);

	virtual ~PolyMesh_() {};

	void create(int numberOfVertices = 0, int numberOfFaces = 0, VertexSize sizePerVertex = VERTEX_SIZE_XYZNT, EdgeNum edgesPerFace = EDGE_NUM_TRI,
		unsigned char flag = VERTEX_HAS_NORMAL | VERTEX_HAS_UV)
	{
		//by Jia: change uchar to unsigned char to fix error: use of undeclared identifier 'uchar'
		_vertexSize = (unsigned char)sizePerVertex;
		_edgeNum = (unsigned char)edgesPerFace;
		_vertexFlag = flag;
		resizeVertices(numberOfVertices);
		resizeFaces(numberOfFaces);
        _coordSpace = CS_UNKNOWN;
	}

	//const T* getVerticesPtr() const { return !_vertices.empty()?&_vertices[0]:NULL; };
	//const VertexIndex* getFacesPtr() const { return !_faces.empty() ? &_faces[0] : NULL; };
	//const std::vector<T>& getVertices() const { return _vertices; }
	//const std::vector<VertexIndex>& getFaces() const { return _faces; }

	void resizeVertices(int numberOfVertices, int texOnlyVertices=0) {
		_vertexCount = numberOfVertices;
		_texVertCount = texOnlyVertices;
		if (_vertices.size() != _vertexSize*_vertexCount) {
			_vertices.resize(_vertexSize*_vertexCount);
			//if (!_vertices.empty())
			//	_verticesPtr = &_vertices[0];
			//else
			//	_verticesPtr = NULL;
		}

	};
	void resizeFaces(int numberOfFaces) {
		_faceCount = numberOfFaces;
		if (_faces.size() != _edgeNum*_faceCount) {
			_faces.resize(_edgeNum*_faceCount);
			//if (!_faces.empty())
			//	_facesPtr = &_faces[0];
			//else
			//	_facesPtr = NULL;
		}
	};

	// Set a vertext data record
	void setVertexData(int vertIndex, T* vertData) {
		T* vPtr = &_vertices[vertIndex*_vertexSize];
		for (int i = 0; i < _vertexSize; i++)
			*vPtr++ = *vertData++;
	};

	// Set a face data record
	void setFaceData(int faceIndex, VertexIndex* faceData) {
		VertexIndex* fPtr = &_faces[faceIndex*_edgeNum];
		for (int i = 0; i < _edgeNum; i++)
			*fPtr++ = *faceData++;
	};

	// Set the validity flag for the vertex data (normal and uv)
	// This controls if a vertext data field will be written to a file 
	void setVertexFlag(unsigned char flag) { _vertexFlag = flag; }
	unsigned char getVertexFlag() const { return _vertexFlag; }

	// These are used to directly access the vertices and faces for performance reason
	// They don't work for full head meshes
	// Use getVertices and getFaces instead
	std::vector<T>& vertices() { return _vertices;  }
	std::vector<VertexIndex>& faces() { return _faces;  }

	// read-only access to vertices and faces
	const std::vector<T>& vertices_r() const { return _vertices; }
	const std::vector<VertexIndex>& faces_r() const { return _faces; }

	// Get a list of vertices of vertexType
	// VERTEX_ALL will return (xyz, normal, uv) for each vertex
	// VERTEX_XYZ, VERTEX_NORMAL, VERTEX_UV will return xyz, normal, and uv respectively
	std::vector<T> getVertices(VertexType vertexType= VERTEX_ALL) const;

    // Return vertices as a list of Point3f
    void getPoints(std::vector<cv::Point3f>& points) const;

	// Rerturn vertex indices to vertex XYZ coordinates (VERTEX_XYZ). Each face contains 3 vertex indices (triangles)
	// VERTEX_NORMAL returns indices to vertex normals.
	// VERTEX_UV returns indices to UV vertices.
	std::vector<int> getFaces(VertexType vertexType) const;

	// return the number of vertices
	int getVertexCount() const { return _vertexCount;  }
	// return the number of faces
	int getFaceCount() const { return _faceCount; }

	// return the number of coordinates per vertex
	int getVertexSize() const { return _vertexSize; }

	void updateVertices() {
		resizeVertices(_vertexCount);
	};
	void updateFaces() {
		resizeFaces(_faceCount);
	};

    // Transform vertices by xf
    void transformVertices(const trimesh::xform& xf);

	// Mesh simplification
	// dispacementMap is a gray scale displacement map (>127 positive, <127 negative offsets) of the mesh, 
	// and is used to guide the simplication to preserve details (where displacement values are high)
	bool simplifyMesh(double targetRatio, const cv::Mat& dispacementMap = cv::Mat());

    bool computeNormals();

    bool empty() const { return _vertexCount == 0; }

    void clear() { _vertices.clear();  _faces.clear(); _vertexCount = _faceCount = 0; }

    void setCoordSpace(CoordSpace cs) { _coordSpace = cs; }
    CoordSpace getCoordSpace() const { return _coordSpace; }
    static cv::String coordSpaceToString(CoordSpace cs);

    cv::Point3f getCentroid() const;

	// Read a file of type OBJ, STL, 3DS, and PLY (bin and ascii)
	// .zip is supported if it contains one of the above file types in the top level folder
	// tempPath must be provided and writable to read a zip file (the file will be unzipped to tempPath first)
	bool readFile(const cv::String& inputFile, const cv::String& tempPath="");

	// materialNameName and material name are used for adding texture map properties to the obj file
    // if textureMap is not empty, also write the textureMap to textureFile as JPEG image
	bool writeObj(const cv::String& outputFile, const cv::String& headerText = "", 
		const cv::String& materialFileName = "", const cv::String& materialName = "",
		bool addLogo=false, double logoSize=0, const cv::String& logoMatName = "material_2") const;

	bool writeStl(const cv::String& outputFile) const;

    // Set binary to false to write ascii, or true to write binary file
    // if textureMap is not empty, also write the textureMap to textureFile as JPEG image
	bool writePly(const cv::String& outputFile, const cv::String& textureFile, bool binary=false, const cv::Mat& textureMap=cv::Mat()) const;

	// write GLTF file
	// Not supported on Android yet because of some gcc compiler errors (return false)
	bool writeGltf(const cv::String& outputFile, const cv::Mat& textureMap=cv::Mat(), 
		bool addLogo = false, const cv::Mat& logoImage=cv::Mat(), float logoSize = 0, bool binary = false) const;

	// export the mesh to the output dir with outputFile name
	// export file type is determined by the outputFile extension
	// if textureMap is not, save the texture map too
	// additional files may be saved to the output dir for obj and ply files
	bool writeFile(const cv::String& outputDir, const cv::String& outputFile, const cv::Mat& textureMap = cv::Mat());

protected:

    bool readAsciiStl(const cv::String& fileName);

    CoordSpace _coordSpace;     // coordinate space of the points
	int _vertexCount;			// number of vertices
	int _faceCount;				// number of faces
	int _texVertCount;			// number of texture only vertices (these are appended to the end of vertices and only U,V coords are valid)
	unsigned char _vertexSize;			// number of items in each vertex (can be 3, 6, or 8)
	unsigned char _edgeNum;				// number of edges in each face (3 for triangles and 4 for quads)
	unsigned char _vertexFlag;
	char _reserved;				// byte padding

	std::vector<T> _vertices;	// container for the vertices
	std::vector<VertexIndex> _faces;	// container for the faces
    std::vector<uchar> _vertexColors;   // rgb color for the vertices (3 bytes per vertex)

private:

	//bool write_obj(const cv::String& fileName, const cv::String& headerText, 
	//	const cv::String& materialFileName, const cv::String& materialName,
	//	bool addLogo, double logoSize, const cv::String& logoMatName) const;
	//bool read_obj(const cv::String& fileName);

	//bool write_stl(const cv::String& fileName) const;

    //bool read_stl(const cv::String& fileName);

};

typedef PolyMesh_<float> PolyMesh;

// Not all functions are implemented for PolyMeshd
typedef PolyMesh_<double> PolyMeshd;



} // namespace b3di