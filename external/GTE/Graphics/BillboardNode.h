// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Node.h>

namespace gte
{
    class BillboardNode : public Node
    {
    public:
        // The model space of the billboard has an up vector of (0,1,0) that
        // is chosen to be the billboard's axis of rotation.

        // Construction.
        BillboardNode(std::shared_ptr<Camera> const& camera);

        // The camera to which the billboard is aligned.
        inline void AlignTo(std::shared_ptr<Camera> const& camera)
        {
            mCamera = camera;
        }

    protected:
        // Support for the geometric update.
        virtual void UpdateWorldData(double applicationTime) override;

        std::shared_ptr<Camera> mCamera;
    };
}
