// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/BitHacks.h>
#include <Mathematics/ContOrientedBox3.h>

// The depth of a node in a (nonempty) tree is the distance from the node to
// the root of the tree.  The height is the maximum depth.  A tree with a
// single node has height 0.  The set of nodes of a tree with the same depth
// is refered to as a level of a tree (corresponding to that depth).  A
// complete binary tree of height H has 2^{H+1}-1 nodes.  The level
// corresponding to depth D has 2^D nodes, in which case the number of
// leaf nodes (depth H) is 2^H.

namespace gte
{
    template <typename Real>
    class OBBTreeForPoints
    {
    public:
        struct Node
        {
            Node()
                :
                depth(0),
                minIndex(std::numeric_limits<uint32_t>::max()),
                maxIndex(std::numeric_limits<uint32_t>::max()),
                leftChild(std::numeric_limits<uint32_t>::max()),
                rightChild(std::numeric_limits<uint32_t>::max())
            {
            }

            OrientedBox3<Real> box;
            uint32_t depth;
            uint32_t minIndex, maxIndex;
            uint32_t leftChild, rightChild;
        };


        // The 'points' array is a collection of vertices, each occupying a
        // chunk of memory with 'stride' bytes.  A vertex must start at the
        // first byte of this chunk but does not necessarily fill it.  The
        // 'height' specifies the height of the tree and must be no larger
        // than 31.  If it is set to std::numeric_limits<uint32_t>::max(),
        // then the entire tree is built and the actual height is computed
        // from 'numPoints'.
        OBBTreeForPoints(uint32_t numPoints, char const* points, size_t stride,
            uint32_t height = std::numeric_limits<uint32_t>::max())
            :
            mNumPoints(numPoints),
            mPoints(points),
            mStride(stride),
            mHeight(height),
            mPartition(numPoints)
        {
            // The tree nodes are indexed by 32-bit unsigned integers, so
            // the number of nodes can be at most 2^{32} - 1.  This limits
            // the height to 31.

            uint32_t numNodes;
            if (mHeight == std::numeric_limits<uint32_t>::max())
            {
                uint32_t minPowerOfTwo =
                    static_cast<uint32_t>(BitHacks::RoundUpToPowerOfTwo(mNumPoints));
                mHeight = BitHacks::Log2OfPowerOfTwo(minPowerOfTwo);
                numNodes = 2 * mNumPoints - 1;
            }
            else
            {
                // The maximum level cannot exceed 30 because we are storing
                // the indices into the node array as 32-bit unsigned
                // integers.
                if (mHeight < 32)
                {
                    numNodes = (uint32_t)(1ULL << (mHeight + 1)) - 1;
                }
                else
                {
                    // When the precondition is not met, return a tree of
                    // height 0 (a single node).
                    mHeight = 0;
                    numNodes = 1;
                }
            }

            // The tree is built recursively.  A reference to a Node is
            // passed to BuildTree and nodes are appended to a std::vector.
            // Because the references are on the stack, we must guarantee no
            // reallocations to avoid invalidating the references.  TODO:
            // This design can be modified to pass indices to the nodes so
            // that reallocation is not a problem.
            mTree.reserve(numNodes);

            // Build the tree recursively.  The array mPartition stores the
            // indices into the 'points' array so that at a node, the points
            // represented by the node are those indexed by the range
            // [node.minIndex, node.maxIndex].
            for (uint32_t i = 0; i < mNumPoints; ++i)
            {
                mPartition[i] = i;
            }
            mTree.push_back(Node());
            BuildTree(mTree.back(), 0, mNumPoints - 1);
        }

        // Member access.
        inline uint32_t GetNumPoints() const
        {
            return mNumPoints;
        }

        inline char const* GetPoints() const
        {
            return mPoints;
        }

        inline size_t GetStride() const
        {
            return mStride;
        }

        inline std::vector<Node> const& GetTree() const
        {
            return mTree;
        }

        inline uint32_t GetHeight() const
        {
            return mHeight;
        }

        inline std::vector<uint32_t> const& GetPartition() const
        {
            return mPartition;
        }

    private:
        inline Vector3<Real> GetPosition(uint32_t index) const
        {
            return *reinterpret_cast<Vector3<Real> const*>(mPoints + index * mStride);
        }

        void BuildTree(Node& node, uint32_t i0, uint32_t i1)
        {
            node.minIndex = i0;
            node.maxIndex = i1;

            if (i0 == i1)
            {
                // We are at a leaf node.  The left and right child indices
                // were set to std::numeric_limits<uint32_t>::max() during
                // construction.

                // Create a degenerate box whose center is the point.
                node.box.center = GetPosition(mPartition[i0]);
                node.box.axis[0] = Vector3<Real>{ (Real)1, (Real)0, (Real)0 };
                node.box.axis[1] = Vector3<Real>{ (Real)0, (Real)1, (Real)0 };
                node.box.axis[2] = Vector3<Real>{ (Real)0, (Real)0, (Real)1 };
                node.box.extent = Vector3<Real>{ (Real)0, (Real)0, (Real)0 };
            }
            else  // i0 < i1
            {
                // We are at an interior node.  Compute an oriented bounding
                // box.
                ComputeOBB(i0, i1, node.box);

                if (node.depth == mHeight)
                {
                    return;
                }

                // Use the box axis corresponding to largest extent for the
                // splitting axis.  Partition the points into two subsets, one
                // for the left child and one for the right child. The subsets
                // have numbers of elements that differ by at most 1, so the
                // tree is balanced.
                Vector3<Real> axis2 = node.box.axis[2];
                uint32_t j0, j1;
                SplitPoints(i0, i1, j0, j1, node.box.center, axis2);

                node.leftChild = static_cast<uint32_t>(mTree.size());
                node.rightChild = node.leftChild + 1;
                mTree.push_back(Node());
                Node& leftTree = mTree.back();
                mTree.push_back(Node());
                Node& rightTree = mTree.back();
                leftTree.depth = node.depth + 1;
                rightTree.depth = node.depth + 1;
                BuildTree(leftTree, i0, j0);
                BuildTree(rightTree, j1, i1);
            }
        }

        void ComputeOBB(uint32_t i0, uint32_t i1, OrientedBox3<Real>& box)
        {
            // Compute the mean of the points.
            Vector3<Real> zero{ (Real)0, (Real)0, (Real)0 };
            box.center = zero;
            for (uint32_t i = i0; i <= i1; ++i)
            {
                box.center += GetPosition(mPartition[i]);
            }
            Real invSize = ((Real)1) / (Real)(i1 - i0 + 1);
            box.center *= invSize;

            // Compute the covariance matrix of the points.
            Real covar00 = (Real)0, covar01 = (Real)0, covar02 = (Real)0;
            Real covar11 = (Real)0, covar12 = (Real)0, covar22 = (Real)0;
            for (uint32_t i = i0; i <= i1; ++i)
            {
                Vector3<Real> diff = GetPosition(mPartition[i]) - box.center;
                covar00 += diff[0] * diff[0];
                covar01 += diff[0] * diff[1];
                covar02 += diff[0] * diff[2];
                covar11 += diff[1] * diff[1];
                covar12 += diff[1] * diff[2];
                covar22 += diff[2] * diff[2];
            }
            covar00 *= invSize;
            covar01 *= invSize;
            covar02 *= invSize;
            covar11 *= invSize;
            covar12 *= invSize;
            covar22 *= invSize;

            // Solve the eigensystem and use the eigenvectors for the box
            // axes.
            SymmetricEigensolver3x3<Real> es;
            std::array<Real, 3> eval;
            std::array<std::array<Real, 3>, 3> evec;
            es(covar00, covar01, covar02, covar11, covar12, covar22, false, +1, eval, evec);
            for (int i = 0; i < 3; ++i)
            {
                box.axis[i] = evec[i];
            }
            box.extent = eval;

            // Let C be the box center and let U0, U1, and U2 be the box axes.
            // Each input point is of the form X = C + y0*U0 + y1*U1 + y2*U2.
            // The following code computes min(y0), max(y0), min(y1), max(y1),
            // min(y2), and max(y2).  The box center is then adjusted to be
            //   C' = C + 0.5*(min(y0)+max(y0))*U0 + 0.5*(min(y1)+max(y1))*U1
            //        + 0.5*(min(y2)+max(y2))*U2
            Vector3<Real> pmin = zero, pmax = zero;
            for (uint32_t i = i0; i <= i1; ++i)
            {
                Vector3<Real> diff = GetPosition(mPartition[i]) - box.center;
                for (int j = 0; j < 3; ++j)
                {
                    Real dot = Dot(diff, box.axis[j]);
                    if (dot < pmin[j])
                    {
                        pmin[j] = dot;
                    }
                    else if (dot > pmax[j])
                    {
                        pmax[j] = dot;
                    }
                }
            }

            Real const half(0.5);
            for (int j = 0; j < 3; ++j)
            {
                box.center += (half * (pmin[j] + pmax[j])) * box.axis[j];
                box.extent[j] = half * (pmax[j] - pmin[j]);
            }
        }

        void SplitPoints(uint32_t i0, uint32_t i1, uint32_t& j0, uint32_t& j1,
            Vector3<Real> const& origin, Vector3<Real> const& direction)
        {
            // Project the points onto the splitting axis.
            uint32_t numProjections = i1 - i0 + 1;
            std::vector<ProjectionInfo> info(numProjections);
            uint32_t i, k;
            for (i = i0, k = 0; i <= i1; ++i, ++k)
            {
                Vector3<Real> diff = GetPosition(mPartition[i]) - origin;
                info[k].pointIndex = mPartition[i];
                info[k].projection = Dot(direction, diff);
            }

            // Partition the projections by the median.
            uint32_t medianIndex = (numProjections - 1) / 2;
            std::nth_element(info.begin(), info.begin() + medianIndex, info.end());

            // Partition the points by the median.
            for (k = 0, j0 = i0 - 1; k <= medianIndex; ++k)
            {
                mPartition[++j0] = info[k].pointIndex;
            }
            for (j1 = i1 + 1; k < numProjections; ++k)
            {
                mPartition[--j1] = info[k].pointIndex;
            }
        }

        struct ProjectionInfo
        {
            ProjectionInfo()
                :
                pointIndex(0),
                projection((Real)0)
            {
            }

            bool operator< (ProjectionInfo const& info) const
            {
                return projection < info.projection;
            }

            uint32_t pointIndex;
            Real projection;
        };

        uint32_t mNumPoints;
        char const* mPoints;
        size_t mStride;
        uint32_t mHeight;
        std::vector<Node> mTree;
        std::vector<uint32_t> mPartition;
    };
}
