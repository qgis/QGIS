// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.01.08

#pragma once

#include <Graphics/Buffer.h>
#include <Graphics/MemberLayout.h>
#include <Graphics/Texture.h>
#include <Mathematics/Logger.h>
#include <algorithm>

// TextureBuffer is currently supported only in the DirectX graphics engine.

namespace gte
{
    class TextureBuffer : public Buffer
    {
    public:
        // Construction.  The HLSL shader contains
        //   tbuffer MyTBuffer { type myArray[numElements]; }
        // where 'type' is a native type such as 'float4' and the texture
        // buffer 'format' specifies how the type is interpreted.  The
        // 'numElements' of the constructor must match that of myArray[].
        // Effectively, the tbuffer is a 1D texture.  If you want to update
        // the contents of the texture buffer at run time, much like you
        // update a constant buffer, set 'allowDynamicUpdate' to 'true';
        // otherwise, the buffer is immutable.
        TextureBuffer(DFType format, unsigned int numElements, bool allowDynamicUpdate);

        DFType GetFormat() const;

        // Access to texture buffer members using the layout of a shader
        // program itself is allowed as long as you have attached the constant
        // buffer to a shader first.
        //   std::shared_ptr<VertexShader> vs = <some shader>;
        //   std::shared_ptr<TextureBuffer> tb = <buffer for the shader>;
        //   vs->Set("MyTBuffer", tb);
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
            LogAssert(iter->numElements == 0, "Member is an array, use GetMember(name,index,value).");
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
            LogAssert(iter->numElements == 0, "Member is an array, use GetMember(name,index,value).");
            LogAssert(iter->offset + sizeof(T) <= mNumBytes, "Reading will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset);
            value = *target;
        }

        // Set or get an array member.
        template <typename T>
        bool SetMember(std::string const& name, unsigned int index, T const& value)
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements > 0, "Member is a singleton, use GetMember(name,value).");
            LogAssert(index < iter->numElements, "Index is out of range for the member array.");
            LogAssert(iter->offset + (index + 1) * sizeof(T) <= mNumBytes, "Writing will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset + index * sizeof(T));
            *target = value;
            return true;
        }

        template <typename T>
        bool GetMember(std::string const& name, unsigned int index, T& value) const
        {
            auto iter = std::find_if(mLayout.begin(), mLayout.end(),
                [&name](MemberLayout const& item) { return name == item.name; });

            LogAssert(iter != mLayout.end(), "Failed to find member name " + name + ".");
            LogAssert(iter->numElements > 0, "Member is a singleton, use GetMember(name,value).");
            LogAssert(index < iter->numElements, "Index is out of range for the member array.");
            LogAssert(iter->offset + (index + 1) * sizeof(T) <= mNumBytes, "Reading will access memory outside the buffer.");

            T* target = reinterpret_cast<T*>(mData + iter->offset + index * sizeof(T));
            value = *target;
            return true;
        }

    protected:
        DFType mFormat;
        std::vector<MemberLayout> mLayout;

    public:
        // For use by the Shader class for storing reflection information.
        static int const shaderDataLookup = 1;
    };
}
