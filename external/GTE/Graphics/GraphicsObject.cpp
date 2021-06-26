// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/GraphicsObject.h>
using namespace gte;

GraphicsObject::~GraphicsObject()
{
    msLFDMutex.lock();
    {
        for (auto listener : msLFDSet)
        {
            listener->OnDestroy(this);
        }
    }
    msLFDMutex.unlock();
}

GraphicsObject::GraphicsObject()
    :
    mType(GT_GRAPHICS_OBJECT)
{
}

GraphicsObject::GraphicsObject(GraphicsObjectType type)
    :
    mType(type)
{
}

void GraphicsObject::SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.insert(listener);
    }
    msLFDMutex.unlock();
}

void GraphicsObject::UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener)
{
    msLFDMutex.lock();
    {
        msLFDSet.erase(listener);
    }
    msLFDMutex.unlock();
}

std::mutex GraphicsObject::msLFDMutex;
std::set<std::shared_ptr<GraphicsObject::ListenerForDestruction>> GraphicsObject::msLFDSet;
