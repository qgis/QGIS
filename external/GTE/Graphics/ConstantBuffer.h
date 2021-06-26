// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/MemberLayout.h>
#include <Mathematics/Logger.h>
#include <algorithm>

namespace gte
{
    class ConstantBuffer : public Buffer
    {
    public:
        // Construction.
        ConstantBuffer(size_t numBytes, bool allowDynamicUpdate);

        // Access to constant buffer members using the layout of a shader
        // program itself is allowed as long as you have attached the constant
        // buffer to a shader first.
        //   std::shared_ptr<VertexShader> vsr = <some shader>;
        //   std::shared_ptr<ConstantBuffer> cb = <buffer for the shader>;
        //   vs->Set("MyCBuffer", cb);
        // Now you can use SetMember/GetMember calls successfully.  In these
        // calls, you are required to specify the correct type T for the
        // member.  No checking is performed for the size of the input; that
        // is, too large a value will cause a memory overwrite within the
        // buffer.  The code does test to ensure that no overwrite occurs
        // outside the buffer.

        inline void SetLayout(std::vector<MemberLayout> const& layout)
        {
            mLayout = layout;
        }

        inline std::vector<MemberLayout> const& GetLayout() const
        {
            return mLayout;
        }

        // Test for existence of a member with the specified name.
        bool HasMember(std::string const& name) const;

        // Set or get a non-array member.
        template <typename T>
        void SetMember(std::string const& name, T const& value)
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements == 0, "Member is an array, use SetMember(name,index,value).");
            LogAssert(iter->offset + sizeof(T) <= mNumBytes, "Writing will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset);
            *target = value;
        }

        template <typename T>
        void GetMember(std::string const& name, T& value) const
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements == 0, "Member is an array, use SetMember(name,index,value).");
            LogAssert(iter->offset + sizeof(T) <= mNumBytes, "Reading will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset);
            value = *target;
        }

        // Set or get an array member.
        template <typename T>
        void SetMember(std::string const& name, unsigned int index, T const& value)
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements > 0, "Member is a singleton, use SetMember(name,value).");
            LogAssert(index < iter->numElements, "Index is out of range for the member array.");
            LogAssert(iter->offset + (index + 1) * sizeof(T) <= mNumBytes, "Writing will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset + index * sizeof(T));
            *target = value;
        }

        template <typename T>
        void GetMember(std::string const& name, unsigned int index, T& value) const
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements > 0, "Member is a singleton, use SetMember(name,value).");
            LogAssert(index < iter->numElements, "Index is out of range for the member array.");
            LogAssert(iter->offset + (index + 1) * sizeof(T) <= mNumBytes, "Reading will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset + index * sizeof(T));
            value = *target;
        }

    protected:
        // Direct3D 11 requires the storage to be a multiple of 16 bytes and
        // Direct3D 12 requires the storage to be a multiple of 256 bytes.
        // This function rounds to the nearest multiple of 256 bytes when
        // allocating buffer memory.
        enum
        {
            CBUFFER_REQUIRED_MINIMUM_BYTES = 256
        };

        static size_t GetRoundedNumBytes(size_t numBytes);

        std::vector<MemberLayout> mLayout;

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 0;
    };
}
