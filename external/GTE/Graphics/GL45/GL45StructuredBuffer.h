// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/StructuredBuffer.h>
#include <Graphics/GL45/GL45Buffer.h>

namespace gte
{
    class GL45StructuredBuffer : public GL45Buffer
    {
    public:
        // Construction.
        virtual ~GL45StructuredBuffer() = default;
        GL45StructuredBuffer(StructuredBuffer const* cbuffer);
        static std::shared_ptr<GEObject> Create(void* unused, GraphicsObject const* object);

        // Member access.
        inline StructuredBuffer* GetStructuredBuffer() const
        {
            return static_cast<StructuredBuffer*>(mGTObject);
        }

        // Bind the structured buffer data to the specified shader storage
        // buffer unit.
        void AttachToUnit(GLint shaderStorageBufferUnit);

        // Copy the counter value to/from another buffer.
        bool CopyCounterValueToBuffer(GL45Buffer* targetBuffer, GLint offset);
        bool CopyCounterValueFromBuffer(GL45Buffer* sourceBuffer, GLint offset);

        // The number of active elements is read from GPU and stored in the
        // StructuredBuffer object  (of type CT_APPEND_CONSUME or CT_COUNTER).
        // The result can be accessed from this object via its member function
        // GetNumActiveElements().
        bool GetNumActiveElements();

        // The number of active elements is written to GPU from the
        // StructuredBuffer object (of type CT_APPEND_CONSUME or CT_COUNTER).
        // The number of active elements can be modified in the
        // StructuredBuffer object via its member function
        // SetNumActiveElements.
        bool SetNumActiveElements();

        // Override needed to handle case of a counter attached so that
        // counter value can be updated first to know how many elements to
        // copy back from GPU.
        virtual bool CopyGpuToCpu() override;

    protected:
        // Must be called by constructor.
        virtual void Initialize() override;

    private:
        GLint mCounterOffset;
    };
}
