// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#if defined(GTE_USE_MSWINDOWS)
#include <Applications/MSW/Window.h>
#endif
#if defined(GTE_USE_LINUX)
#include <Applications/GLX/Window.h>
#endif
#include <Applications/CameraRig.h>
#include <Applications/TrackBall.h>
#include <Graphics/PVWUpdater.h>

namespace gte
{
    class Window3 : public Window
    {
    protected:
        // Abstract base class.
        Window3(Parameters& parameters);

    public:
        // Create the camera and camera rig.
        void InitializeCamera(float upFovDegrees, float aspectRatio, float dmin, float dmax,
            float translationSpeed, float rotationSpeed, std::array<float, 3> const& pos,
            std::array<float, 3> const& dir, std::array<float, 3> const& up);

        // The camera frustum is modified.  Any subscribers to the pvw-matrix
        // update system of the camera rig must be updated.  No call is made
        // to OnDisplay() or OnIdle().  The base class is unaware of which
        // display method you use, so to have a visual update you must
        // override OnResize
        //    bool MyApplication::OnResize(int xSize, int ySize)
        //    {
        //        if (Window3::OnResize(xSize, ySize))
        //        {
        //            OnIdle();  // or OnDisplay() or MyOwnDrawFunction() ...
        //        }
        //    }
        virtual bool OnResize(int xSize, int ySize) override;

        // The key 't' decreases the translation speed and the 'T' key
        // increases the translation speed.  The 'r' key decreases the
        // rotation speed and the 'R' key increases the rotation speed.
        virtual bool OnCharPress(unsigned char key, int x, int y) override;

        // The appropriate camera rig motion is selected when 'key' is mapped
        // to a camera motion.
        virtual bool OnKeyDown(int key, int x, int y) override;
        virtual bool OnKeyUp(int key, int x, int y) override;

        // Control the rotation of the trackball.
        virtual bool OnMouseClick(MouseButton button, MouseState state,
            int x, int y, unsigned int modifiers) override;

        virtual bool OnMouseMotion(MouseButton button, int x, int y,
            unsigned int modifiers) override;

    protected:
        BufferUpdater mUpdater;
        std::shared_ptr<Camera> mCamera;
        CameraRig mCameraRig;
        PVWUpdater mPVWMatrices;
        TrackBall mTrackBall;
    };
}
