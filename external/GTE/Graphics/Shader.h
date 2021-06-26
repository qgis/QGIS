// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ConstantBuffer.h>
#include <Graphics/RawBuffer.h>
#include <Graphics/SamplerState.h>
#include <Graphics/StructuredBuffer.h>
#include <Graphics/Texture1.h>
#include <Graphics/Texture2.h>
#include <Graphics/Texture3.h>
#include <Graphics/TextureBuffer.h>
#include <Graphics/TextureCube.h>
#include <Graphics/Texture1Array.h>
#include <Graphics/Texture2Array.h>
#include <Graphics/TextureCubeArray.h>
#include <Mathematics/Logger.h>

namespace gte
{
    class Shader : public GraphicsObject
    {
    public:
        // Abstract base class.  A derived class Shader is implemented for
        // each of the DirectX and OpenGL engines.
        Shader(GraphicsObjectType type);

        // To avoid frequent string comparisons during run time, obtain a
        // handle for an object and use it instead for setting and getting the
        // values.  If the named object exists, the returned handle is
        // nonnegative; otherwise, it is -1.
        int Get(std::string const& name) const;

        // Set or get the buffers.  If the set is successful, the return value
        // is nonnegative and is the index into the appropriate array.  This
        // handle may be passed to the Set(handle,*) and Get(handle,*)
        // functions.  The mechanism allows you to set directly by index and
        // avoid the name comparisons that occur with the Set(name,*) and
        // Get(name,*) functions.
        template <typename T>
        int Set(std::string const& name, std::shared_ptr<T> const& object)
        {
            int handle = 0;
            for (auto& data : mData[T::shaderDataLookup])
            {
                if (name == data.name)
                {
                    if (IsValid(data, object.get()))
                    {
                        data.object = object;
                        return handle;
                    }
                    return -1;
                }
                ++handle;
            }

            LogError("Cannot find object " + name + ".");
        }

        template <typename T>
        std::shared_ptr<T> const Get(std::string const& name) const
        {
            for (auto const& data : mData[T::shaderDataLookup])
            {
                if (name == data.name)
                {
                    return std::static_pointer_cast<T>(data.object);
                }
            }
            return nullptr;
        }

        template <typename T>
        void Set(int handle, std::shared_ptr<T> const& object)
        {
            std::vector<Data>& data = mData[T::shaderDataLookup];
            if (0 <= handle && handle < static_cast<int>(data.size()))
            {
                auto& d = data[handle];
                if (IsValid(d, object.get()))
                {
                    d.object = object;
                }
                return;
            }

            LogError("Invalid handle for object.");
        }

        template <typename T>
        std::shared_ptr<T> const Get(int handle) const
        {
            std::vector<Data> const& data = mData[T::shaderDataLookup];
            if (0 <= handle && handle < static_cast<int>(data.size()))
            {
                return std::static_pointer_cast<T>(data[handle].object);
            }
            return nullptr;
        }

        // Set the texture and sampler at the same time.  This hides the
        // difference between HLSL and GLSL.  HLSL has both Texture* objects
        // and SamplerState objects.  GLSL combines these into a sampler2D
        // object.  The textureName and samplerName are both used by HLSL.
        // The samplerName is used for the GLSL sampler2D and the textureName
        // is ignored.  The GTEngine sample applications are created to
        // compile using either DX11 or OpenGL.  If you set up your
        // application the same way, you will need to be consistent in your
        // naming of HLSL Texture* objects and GLSL sampler2D objects.  Even
        // though a SamplerState object can be shared by multiple textures in
        // an HLSL program, you will need to declare a SamplerState per
        // Texture* object.
        virtual void Set(std::string const& textureName,
            std::shared_ptr<TextureSingle> const& texture,
            std::string const& samplerName,
            std::shared_ptr<SamplerState> const& state) = 0;

        virtual void Set(std::string const& textureName,
            std::shared_ptr<TextureArray> const& texture,
            std::string const& samplerName,
            std::shared_ptr<SamplerState> const& state) = 0;

        // Access size of one of these buffers.
        // Returns 0 if the requested buffer does not exist.
        // For StructuredBuffers, it's the size of one structure element.
        unsigned int GetConstantBufferSize(int handle) const;
        unsigned int GetConstantBufferSize(std::string const& name) const;
        unsigned int GetTextureBufferSize(int handle) const;
        unsigned int GetTextureBufferSize(std::string const& name) const;
        unsigned int GetStructuredBufferSize(int handle) const;
        unsigned int GetStructuredBufferSize(std::string const& name) const;

        // Access member layouts for these types of buffers.  Only GL45
        // needs access to the structured buffer layout.
        void GetConstantBufferLayout(int handle, BufferLayout& layout) const;
        void GetConstantBufferLayout(std::string const& name, BufferLayout& layout) const;
        void GetTextureBufferLayout(int handle, BufferLayout& layout) const;
        void GetTextureBufferLayout(std::string const& name, BufferLayout& layout) const;
        void GetStructuredBufferLayout(int handle, BufferLayout& layout) const;
        void GetStructuredBufferLayout(std::string const& name, BufferLayout& layout) const;

        inline unsigned int GetNumXThreads() const
        {
            return mNumXThreads;
        }

        inline unsigned int GetNumYThreads() const
        {
            return mNumYThreads;
        }

        inline unsigned int GetNumZThreads() const
        {
            return mNumZThreads;
        }

        // Names for the 'struct Data' defined in the private section below.
        // Only GL45 requires the atomic counter names.  Only DX11 requires
        // the raw and typed buffer names.
        enum
        {
            ConstantBufferShaderDataLookup = 0,         // CB
            TextureBufferShaderDataLookup = 1,          // TB
            StructuredBufferShaderDataLookup = 2,       // SB
            RawBufferShaderDataLookup = 3,              // RB
            TypedBufferShaderDataLookup = 4,            // TY
            TextureSingleShaderDataLookup = 5,          // TX
            TextureArrayShaderDataLookup = 6,           // TA
            SamplerStateShaderDataLookup = 7,           // SS
            AtomicCounterBufferShaderDataLookup = 8,    // AB
            AtomicCounterShaderDataLookup = 9,          // AC
            NUM_LOOKUP_INDICES = 10
        };

    protected:
        // This structure provides the data necessary for the engine to attach
        // the associated resources to the shader, including name lookups and
        // resource validation.  Only GL45 requires the atomic counter names.
        //   CB - constant buffer, lookup 0
        //   TB - texture buffer, lookup 1
        //   SB - structured buffer (and derived classes), lookup 2
        //   TY - typed buffer, lookup 3 [DX11 only]
        //   RB - raw buffer, lookup 4  [DX11 only]
        //   TX - texture (and derived classes), lookup 5
        //   TA - texture array (and derived classes), lookup 6
        //   SS - sampler state, lookup 7
        //   AB - atomic (counter) buffer, lookup [GL45 only]
        //   AC - atomic counter, lookup 9  [GL45 only]
        //
        // For GL45, here is how to use atomic counter information. Given
        // structured buffer data at index some-index:
        //   sb = mData[StructuredBufferShaderDataLookup][some-index];
        // Does the structured buffer have a counter?  Check
        //   sb.isGpuWritable
        // Access the atomic counter:
        //   ac = mData[AtomicCounterShaderDataLookup][sb.extra];
        // Access where this atomic counter exists in one of the atomic
        // counter buffers:
        //   acb = mData[AtomicCounterBufferShaderDataLookup][ac.bindPoint];
        //   acbIndex = acb.bindPoint;
        //   acbOffset = acb.extra;

        struct Data
        {
            Data(GraphicsObjectType inType, std::string const& inName, int inBindPoint,
                int inNumBytes, unsigned int inExtra, bool inIsGpuWritable)
                :
                type(inType),
                name(inName),
                bindPoint(inBindPoint),
                numBytes(inNumBytes),
                extra(inExtra),
                isGpuWritable(inIsGpuWritable)
            {
            }

            // CB, TB, SB, RB, TY, TX, TA, SS
            std::shared_ptr<GraphicsObject> object;

            // CB, TB, SB, RB, TY, TX, TA, SS
            GraphicsObjectType type;

            // CB, TB, SB, RB, TY, TX, TA, SS, AC
            std::string name;

            // CB, TB, SB, RB, TY, TX, TA, SS, AB, AC
            int bindPoint;

            // CB, TB, SB, RB, TY, AB, AC (always 4)
            int numBytes;

            // TX, TA (dims), SS (type for TX or TA), SB (if has atomic
            // counter, AC index), AC (offset)
            unsigned int extra;

            // SB (true if has atomic counter), RB, TX/TA (in GL45, false
            // for gsampler*, true for gimage*)
            bool isGpuWritable;
        };

        virtual bool IsValid(Data const& goal, ConstantBuffer* resource) const = 0;
        virtual bool IsValid(Data const& goal, TextureBuffer* resource) const = 0;
        virtual bool IsValid(Data const& goal, StructuredBuffer* resource) const = 0;
        virtual bool IsValid(Data const& goal, RawBuffer* resource) const = 0;
        virtual bool IsValid(Data const& goal, TextureSingle* resource) const = 0;
        virtual bool IsValid(Data const& goal, TextureArray* resource) const = 0;
        virtual bool IsValid(Data const& goal, SamplerState* state) const = 0;

        std::vector<Data> mData[NUM_LOOKUP_INDICES];
        std::vector<unsigned char> mCompiledCode;
        unsigned int mNumXThreads;
        unsigned int mNumYThreads;
        unsigned int mNumZThreads;

        std::vector<BufferLayout> mCBufferLayouts;
        std::vector<BufferLayout> mTBufferLayouts;
        std::vector<BufferLayout> mSBufferLayouts;

    public:
        // For use by the graphics engine.
        inline std::vector<unsigned char> const& GetCompiledCode() const
        {
            return mCompiledCode;
        }

        inline std::vector<Shader::Data> const& GetData(int lookup) const
        {
            return mData[lookup];
        }
    };

    // Specialization to copy the member layouts of the shader program to
    // the buffer objects.
    template <>
    inline int Shader::Set(std::string const& name, std::shared_ptr<ConstantBuffer> const& object)
    {
        int handle = 0;
        for (auto& data : mData[ConstantBuffer::shaderDataLookup])
        {
            if (name == data.name)
            {
                if (IsValid(data, object.get()))
                {
                    data.object = object;
                    object->SetLayout(mCBufferLayouts[handle]);
                    return handle;
                }
                return -1;
            }
            ++handle;
        }

        LogError("Cannot find object " + name + ".");
    }

    // Specialization to copy the member layouts of the shader program to
    // the buffer objects.
    template <>
    inline int Shader::Set(std::string const& name, std::shared_ptr<TextureBuffer> const& object)
    {
        int handle = 0;
        for (auto& data : mData[TextureBuffer::shaderDataLookup])
        {
            if (name == data.name)
            {
                if (IsValid(data, object.get()))
                {
                    data.object = object;
                    object->SetLayout(mTBufferLayouts[handle]);
                    return handle;
                }
                return -1;
            }
            ++handle;
        }

        LogError("Cannot find object " + name + ".");
    }

    // Specialization to copy the member layouts of the shader program to
    // the buffer objects.
    template <>
    inline void Shader::Set(int handle, std::shared_ptr<ConstantBuffer> const& object)
    {
        std::vector<Data>& data = mData[ConstantBuffer::shaderDataLookup];
        if (0 <= handle && handle < static_cast<int>(data.size()))
        {
            auto& d = data[handle];
            if (IsValid(d, object.get()))
            {
                d.object = object;
                object->SetLayout(mCBufferLayouts[handle]);
            }
            return;
        }

        LogError("Invalid handle for object.");
    }

    // Specialization to copy the member layouts of the shader program to
    // the buffer objects.
    template <>
    inline void Shader::Set(int handle, std::shared_ptr<TextureBuffer> const& object)
    {
        std::vector<Data>& data = mData[TextureBuffer::shaderDataLookup];
        if (0 <= handle && handle < static_cast<int>(data.size()))
        {
            auto& d = data[handle];
            if (IsValid(d, object.get()))
            {
                d.object = object;
                object->SetLayout(mTBufferLayouts[handle]);
            }
            return;
        }

        LogError("Invalid handle for object.");
    }
}
