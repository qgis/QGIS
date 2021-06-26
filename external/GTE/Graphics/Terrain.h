// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Node.h>
#include <Graphics/Camera.h>
#include <Graphics/MeshFactory.h>
#include <Mathematics/Vector2.h>

namespace gte
{
    class Terrain : public Node
    {
    public:
        // Construction and destruction.  The following preconditions must be
        // satisfied:
        //   1. The array is truly 2-dimensional, NumRows > 0 and NumCols > 0.
        //   2. Size = 2^p + 1 for 1 <= p <= 7.
        //   3. Each tile is a Size-by-Size array of heights stored in
        //      row-major order.
        //   4. The elevation extremes satisfy minElevation <= maxElevation.
        //   5. The tile pixels are square with spacing > 0.
        //   6. The vformat has first Bind call using VA_POSITION.  The data
        //      type can be DF_R32G32B32_FLOAT or DF_R32G32B32A32_FLOAT.  The
        //      unit must be 0.
        //   7. The camera is not null.

        virtual ~Terrain() = default;

        Terrain(size_t numRows, size_t numCols, size_t size, float minElevation,
            float maxElevation, float spacing, VertexFormat const& vformat,
            std::shared_ptr<Camera> camera);

        // Member access.
        inline size_t GetNumRows() const
        {
            return mNumRows;
        }

        inline size_t GetNumCols() const
        {
            return mNumCols;
        }

        inline size_t GetSize() const
        {
            return mSize;
        }

        inline float GetMinElevation() const
        {
            return mMinElevation;
        }

        inline float GetMaxElevation() const
        {
            return mMaxElevation;
        }

        inline float GetSpacing() const
        {
            return mSpacing;
        }

        std::shared_ptr<Visual> GetPage(size_t row, size_t col) const;

        // The heights of all the terrain pages should be set after the
        // terrain constructor is called.  During program execution, the
        // heights can be modified according to the application's needs.
        void SetHeights(size_t row, size_t col, std::vector<unsigned short> const& heights);

        // Get the height array of the specified terrain page.  If the input
        // (row,col) is invalid, return the height array of page (0,0).
        std::vector<unsigned short> const& GetHeights(size_t row, size_t col) const;

        // Compute the terrain height at the world (x,y) coordinate and uses
        // wrap-around when necessary.
        float GetHeight(float x, float y) const;

        // Estimate a normal vector at the world (x,y) coordinate by using the
        // neighbors (x+dx,y+dy), where (dx,dy) in
        // {(s,0),(-s,0),(0,s),(0,-s)}.  The value s is the spacing parameter.
        Vector3<float> GetNormal(float x, float y) const;

        // Update the active set of terrain pages.
        void OnCameraMotion();

    protected:
        class Page : public Visual
        {
        public:
            // Comstruction and destruction.
            virtual ~Page() = default;

            Page(size_t size, float minElevation, float maxElevation,
                float spacing, float length, Vector2<float> const& origin,
                VertexFormat const& vformat);

            // Member access.
            inline Vector2<float> const& GetOrigin() const
            {
                return mOrigin;
            }

            // If the vertex buffer has been copied from CPU to GPU, the
            // caller must re-copy the buffer after a call to setting the
            // heights.
            void SetHeights(std::vector<unsigned short> const& heights);

            inline std::vector<unsigned short> const& GetHeights() const
            {
                return mHeights;
            }

            // Height field measurements.  If the location is not in the page,
            // the return value is std::numeric_limits<float>::max().
            float GetHeight(float x, float y) const;

        private:
            float GetHeight(size_t i) const;
            float GetHeight(size_t row, size_t col) const;

            // Height field parameters.
            size_t mSize;
            float mMinElevation, mMaxElevation, mSpacing;
            Vector2<float> mOrigin;
            std::vector<unsigned short> mHeights;
        };

        std::shared_ptr<Page> GetPage(float x, float y) const;

        // Terrain information.
        size_t mNumRows, mNumCols, mSize;

        // Page information.
        float mMinElevation, mMaxElevation, mSpacing, mLength;

        // Current page containing the camera.
        size_t mCameraRow, mCameraCol;
        std::shared_ptr<Camera> mCamera;
    };
}
