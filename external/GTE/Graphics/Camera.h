// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ViewVolume.h>

namespace gte
{

    class Camera : public ViewVolume
    {
    public:
        // Construction.  The depth range for DirectX is [0,1] and for OpenGL
        // is [-1,1].  For DirectX, set isDepthRangeZeroToOne to true.  For
        // OpenGL, set isDepthRangeZeroOne to false.
        Camera(bool isPerspective, bool isDepthRangeZeroOne);

        // Support for parallax projection. You specify a convex quadrilateral
        // viewport.  The points must be in camera coordinates and are ordered
        // counterclockwise as viewed from the eyepoint.  The plane of the
        // quadrilateral is the view plane and has an "extrude" value of 1.
        // The nearExtrude value is in (0,infinity); this specifies the
        // fraction from the eyepoint to the view plane containing the
        // near-face of the cuboidal view volume.  The farExtrude value is in
        // (nearExtrude,infinity); this specifies the fraction from the
        // eyepoint at which to place the far-face of the cuboidal view
        // volume.
        void SetParallaxProjectionMatrix(Vector4<float> const& p00,
            Vector4<float> const& p10, Vector4<float> const& p11,
            Vector4<float> const& p01, float nearExtrude, float farExtrude);

        // The preview matrix is applied after the model-to-world but before
        // the view matrix.  It is used for transformations such as
        // reflections of world objects.  The default value is the identity
        // matrix.
        void SetPreViewMatrix(Matrix4x4<float> const& preViewMatrix);

        inline Matrix4x4<float> const& GetPreViewMatrix() const
        {
            return mPreViewMatrix;
        }

        inline bool PreViewIsIdentity() const
        {
            return mPreViewIsIdentity;
        }

        // The postprojection matrix is used for screen-space transformations
        // such as reflection of the rendered image.  The default value is the
        // identity matrix.
        void SetPostProjectionMatrix(Matrix4x4<float> const& postProjMatrix);

        inline Matrix4x4<float> const& GetPostProjectionMatrix() const
        {
            return mPostProjectionMatrix;
        }

        inline bool PostProjectionIsIdentity() const
        {
            return mPostProjectionIsIdentity;
        }

        // Compute a picking line from the left-handed screen coordinates
        // (x,y), the viewport, and the camera.  The output 'origin' is the
        // camera position and the 'direction' is a unit-length vector, both
        // in world coordinates.  The return value is 'true' iff (x,y) is in
        // the viewport.
        bool GetPickLine(int viewX, int viewY, int viewW, int viewH, int x, int y,
            Vector4<float>& origin, Vector4<float>& direction) const;

    private:
        // After modifying the pre-view matrix or post-projection matrix,
        // update the projection-view matrix to include these.
        virtual void UpdatePVMatrix();

        // The preview matrix for the camera.
        Matrix4x4<float> mPreViewMatrix;

        // The postprojection matrix for the camera.
        Matrix4x4<float> mPostProjectionMatrix;

        // Indicates whether a user has specified a nonidentity pre-view
        // matrix.
        bool mPreViewIsIdentity;

        // Indicates whether a user has specified a nonidentity
        // post-projection matrix.
        bool mPostProjectionIsIdentity;
    };
}
