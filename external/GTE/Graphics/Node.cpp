// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.10

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Node.h>
#include <Mathematics/Logger.h>
using namespace gte;

Node::~Node()
{
    for (auto& child : mChild)
    {
        if (child)
        {
            child->SetParent(nullptr);
            child = nullptr;
        }
    }
}

int Node::GetNumChildren() const
{
    return static_cast<int>(mChild.size());
}

int Node::AttachChild(std::shared_ptr<Spatial> const& child)
{
    LogAssert(child != nullptr, "You cannot attach null children to a node.");
    LogAssert(child->GetParent() == nullptr, "The child already has a parent.");

    child->SetParent(this);

    // Insert the child in the first available slot (if any).
    int i = 0;
    for (auto& current : mChild)
    {
        if (!current)
        {
            current = child;
            return i;
        }
        ++i;
    }

    // All slots are used, so append the child to the array.
    int const numChildren = static_cast<int>(mChild.size());
    mChild.push_back(child);
    return numChildren;
}

int Node::DetachChild(std::shared_ptr<Spatial> const& child)
{
    if (child)
    {
        int i = 0;
        for (auto& current : mChild)
        {
            if (current == child)
            {
                current->SetParent(nullptr);
                current = nullptr;
                return i;
            }
            ++i;
        }
    }
    return -1;
}

std::shared_ptr<Spatial> Node::DetachChildAt(int i)
{
    if (0 <= i && i < static_cast<int>(mChild.size()))
    {
        std::shared_ptr<Spatial> child = mChild[i];
        if (child)
        {
            child->SetParent(nullptr);
            mChild[i] = nullptr;
        }
        return child;
    }
    return nullptr;
}

void Node::DetachAllChildren()
{
    for (auto& current : mChild)
    {
        DetachChild(current);
    }
}

std::shared_ptr<Spatial> Node::SetChild(int i,
    std::shared_ptr<Spatial> const& child)
{
    LogAssert(child == nullptr || child->GetParent() == nullptr, "The child already has a parent.");

    int const numChildren = static_cast<int>(mChild.size());
    if (0 <= i && i < numChildren)
    {
        // Remove the child currently in the slot.
        std::shared_ptr<Spatial> previousChild = mChild[i];
        if (previousChild)
        {
            previousChild->SetParent(nullptr);
        }

        // Insert the new child in the slot.
        if (child)
        {
            child->SetParent(this);
        }

        mChild[i] = child;
        return previousChild;
    }

    // The index is out of range, so append the child to the array.
    if (child)
    {
        child->SetParent(this);
    }
    mChild.push_back(child);
    return nullptr;
}

std::shared_ptr<Spatial> Node::GetChild(int i)
{
    if (0 <= i && i < static_cast<int>(mChild.size()))
    {
        return mChild[i];
    }
    return nullptr;
}

void Node::UpdateWorldData(double applicationTime)
{
    Spatial::UpdateWorldData(applicationTime);

    for (auto& child : mChild)
    {
        if (child)
        {
            child->Update(applicationTime, false);
        }
    }
}

void Node::UpdateWorldBound()
{
    if (!worldBoundIsCurrent)
    {
        // Start with an invalid bound.
        worldBound.SetCenter({ 0.0f, 0.0f, 0.0f });
        worldBound.SetRadius(0.0f);

        for (auto& child : mChild)
        {
            if (child)
            {
                // GrowToContain ignores invalid child bounds.  If the world
                // bound is invalid and a child bound is valid, the child
                // bound is copied to the world bound.  If the world bound and
                // child bound are valid, the smallest bound containing both
                // bounds is assigned to the world bound.
                worldBound.GrowToContain(child->worldBound);
            }
        }
    }
}

void Node::GetVisibleSet(Culler& culler,
    std::shared_ptr<Camera> const& camera, bool noCull)
{
    for (auto& child : mChild)
    {
        if (child)
        {
            child->OnGetVisibleSet(culler, camera, noCull);
        }
    }
}
