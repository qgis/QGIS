// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>

// IndirectArgumentsBuffer is currently supported only in the DirectX graphics
// engine.

namespace gte
{
    class IndirectArgumentsBuffer : public Buffer
    {
    public:
        // Construction.  Each element is a 4-byte value.  For storing a
        // single set of input parameters to a draw call, the layout of
        // the buffer should be as shown next.  The parameters are in the
        // order expected by the Draw* call without the 'Indirect' suffix.
        //
        // DrawInstancedIndirect:
        //   UINT VertexCountPerInstance;
        //   UINT InstanceCount;
        //   UINT StartVertexLocation;
        //   UINT StartInstanceLocation;
        //
        // DrawIndexedInstancedIndirect:
        //   UINT IndexCountPerInstance;
        //   UINT InstanceCount;
        //   UINT StartIndexLocation;
        //   INT  BaseVertexLocation;
        //   UINT StartInstanceLocation;
        //
        // DispatchIndirect:
        //   UINT ThreadsGroupCountX;
        //   UINT ThreadsGroupCountY;
        //   UINT ThreadsGroupCountZ;

        IndirectArgumentsBuffer(unsigned int numElements, bool createStorage = true);
    };
}
