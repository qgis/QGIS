// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>

namespace gte
{
    class StructuredBuffer : public Buffer
    {
    public:
        // Construction.
        StructuredBuffer(unsigned int numElements, size_t elementSize, bool createStorage = true);

        enum CounterType
        {
            CT_NONE,  // default, 
            CT_APPEND_CONSUME,
            CT_COUNTER
        };

        inline CounterType GetCounterType() const
        {
            return mCounterType;
        }

        // Call one of these functions before binding the buffer to the
        // engine.  These will set the CounterType and set the usage to
        // SHADER_OUTPUT.
        inline void MakeAppendConsume()
        {
            mCounterType = CT_APPEND_CONSUME;
            mUsage = SHADER_OUTPUT;
        }

        inline void MakeCounter()
        {
            mCounterType = CT_COUNTER;
            mUsage = SHADER_OUTPUT;
        }

        // Let the GPU know whether or not to change its internal count when
        // the buffer has a counter (CT_APPEND_CONSUME or CT_COUNTER).  An
        // input of 'true' means the GPU will let the counter keep its current
        // value.  This function is ignored when the buffer has counter type
        // CT_NONE.
        inline void SetKeepInternalCount(bool keepInternalCount)
        {
            if (mCounterType != CT_NONE)
            {
                mKeepInternalCount = keepInternalCount;
            }
        }

        inline bool GetKeepInternalCount() const
        {
            return mKeepInternalCount;
        }

        // To access the active number of elements maintained by the GPU for
        // the CT_APPEND_CONSUME or CT_COUNTER buffers, you must call
        //   DX11Engine* engine = <your engine>;
        //   StructuredBuffer* buf = <your buffer>;
        //   engine->GetNumActiveElements(buf);  // copy count from GPU to CPU
        //   int numElements = buf->GetNumActiveElements();
        // Also
        //   engine->CopyGpuToCpu(buf);
        // will fetch the buffer contents as well as the active number.

    protected:
        CounterType mCounterType;
        bool mKeepInternalCount;

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 2;
    };
}
