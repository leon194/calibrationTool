/*M///////////////////////////////////////////////////////////////////////////////////////
//
//
// Copyright (c) 2018 Bellus3D, Inc. All rights reserved.
//
// 6/01/2018    jingliu created
//
//M*/
#pragma once
#include <memory>

#include <b3ddepth/core/B3DDef.h>

namespace b3dd {

    /**
    * @brief  Possible image types for B3DImage
    */
    enum B3DImageType
    {
        B3DIMAGE_UNCHANGED,     // Unknown type
        B3DIMAGE_RGB,           // 24-bit rgb
        B3DIMAGE_MONO,          // 8-bit gray scale (IR image)
        B3DIMAGE_DEPTH_16,      // 16-bit depth
        B3DIMAGE_DISP_16,       // 16-bit signed value
        B3DIMAGE_DEPTH_32,      // 32-bit floating point depth
    };

    class B3DImageImpl;
    using B3DImageImplPtr = std::shared_ptr<B3DImageImpl>;

    class B3DImage {
    public:
        DLLEXPORT B3DImage();
        DLLEXPORT virtual ~B3DImage();

        /**
        * @brief  Constructor for B3DImage. Image data will be allocated.
        *
        * @param[in] int rows, number of rows in 2D image
        * @param[in] int cols, number of columns in 2D image
        * @param[in] B3DImageType type, image type
        *
        */
        DLLEXPORT B3DImage(int rows, int cols, B3DImageType type);

        /**
        * @brief  Constructor for B3DImage from data pointer directly
        * 
        * This constructor will NOT make a copy of the "data", caller is responsible for releasing it. 
        *
        * @param[in] int rows, number of rows in 2D image
        * @param[in] int cols, number of columns in 2D image
        * @param[in] B3DImageType type, image type
        * @param[in] unsigned char* data, pointer to the image data
        * @param[in] size_t rowSize, number of bytes each matrix row occupies.
        *                            Default is '0' which means continuous.
        */
        DLLEXPORT B3DImage(int rows, int cols, B3DImageType type, unsigned char* data, size_t rowSize = 0);


        DLLEXPORT bool isEmpty() const;

        DLLEXPORT B3DImageType type() const;

        DLLEXPORT int rows() const;

        DLLEXPORT int cols() const;

        DLLEXPORT size_t rowSize() const;

        DLLEXPORT unsigned char* data() const;


        /**
        * @brief  Construct B3DImage directly from its implementation.
        * For internal image conversion convenience.
        *
        * @param[in] B3DImageImplPtr impl, the pointer to B3DImage implementation instance.
        */
        B3DImage(B3DImageImplPtr impl);

        // Return the implementation of B3DImage. For internal use only.
        B3DImageImplPtr getImpl() const;

    protected:
        B3DImageImplPtr _impl;  // For internal use only
    };


}  // End of namespace b3dd
