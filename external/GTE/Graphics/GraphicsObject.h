// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>

namespace gte
{
    // The current hierarchy of graphics objects is
    //  GraphicsObject
    //      Resource
    //          Buffer
    //              ConstantBuffer
    //              TextureBuffer
    //              VertexBuffer
    //              IndexBuffer
    //              StructuredBuffer
    //              TypedBuffer
    //              RawBuffer
    //              IndirectArgumentsBuffer
    //          Texture
    //              TextureSingle
    //                  Texture1
    //                  Texture2
    //                      TextureRT
    //                      TextureDS
    //                  Texture3
    //              TextureArray
    //                  Texture1Array
    //                  Texture2Array
    //                  TextureCube
    //                  TextureCubeArray
    //      ShaderBase
    //          Shader
    //              VertexShader
    //              GeometryShader
    //              PixelShader
    //              ComputeShader
    //      DrawingState
    //          SamplerState
    //          BlendState
    //          DepthStencilState
    //          RasterizerState

    // The GraphicsObjectType enumeration is for run-time type information.
    // The typeid() mechanism does not work in Unbind() calls because the
    // listeners receive 'this' from a base class during a destructor call.
    enum GraphicsObjectType
    {
        GT_GRAPHICS_OBJECT,  // abstract
        GT_RESOURCE,  // abstract
        GT_BUFFER,  // abstract
        GT_CONSTANT_BUFFER,
        GT_TEXTURE_BUFFER,
        GT_VERTEX_BUFFER,
        GT_INDEX_BUFFER,
        GT_STRUCTURED_BUFFER,
        GT_TYPED_BUFFER,
        GT_RAW_BUFFER,
        GT_INDIRECT_ARGUMENTS_BUFFER,
        GT_TEXTURE,  // abstract
        GT_TEXTURE_SINGLE,  // abstract
        GT_TEXTURE1,
        GT_TEXTURE2,
        GT_TEXTURE_RT,
        GT_TEXTURE_DS,
        GT_TEXTURE3,
        GT_TEXTURE_ARRAY,  // abstract
        GT_TEXTURE1_ARRAY,
        GT_TEXTURE2_ARRAY,
        GT_TEXTURE_CUBE,
        GT_TEXTURE_CUBE_ARRAY,
        GT_SHADER,  // abstract
        GT_VERTEX_SHADER,
        GT_GEOMETRY_SHADER,
        GT_PIXEL_SHADER,
        GT_COMPUTE_SHADER,
        GT_DRAWING_STATE,  // abstract
        GT_SAMPLER_STATE,
        GT_BLEND_STATE,
        GT_DEPTH_STENCIL_STATE,
        GT_RASTERIZER_STATE,
        GT_NUM_TYPES
    };

    class GraphicsObject
    {
    public:
        // This is an abstract base class that is used for bridging GTEngine
        // graphics objects with DX11 and GL45 graphics objects.
        virtual ~GraphicsObject();
    protected:
        GraphicsObject();
        GraphicsObject(GraphicsObjectType type);

    public:
        // Run-time type information.
        inline GraphicsObjectType GetType() const
        {
            return mType;
        }

        inline bool IsBuffer() const
        {
            return GT_BUFFER <= mType && mType <= GT_INDIRECT_ARGUMENTS_BUFFER;
        }

        inline bool IsTexture() const
        {
            return GT_TEXTURE_SINGLE <= mType && mType <= GT_TEXTURE3;
        }

        inline bool IsTextureArray() const
        {
            return GT_TEXTURE_ARRAY <= mType && mType <= GT_TEXTURE_CUBE_ARRAY;
        }

        inline bool IsShader() const
        {
            return GT_SHADER <= mType && mType <= GT_COMPUTE_SHADER;
        }

        inline bool IsDrawingState() const
        {
            return GT_DRAWING_STATE <= mType && mType <= GT_RASTERIZER_STATE;
        }

        // Naming support.  The default name is "".
        inline void SetName(std::string const& name)
        {
            mName = name;
        }

        inline std::string const& GetName() const
        {
            return mName;
        }

        // Listeners subscribe to receive notification when a GraphicsObject
        // is about to be destroyed.  The intended use is for the DX11/GL45
        // objects to destroy corresponding graphics-API-specific objects.
        class ListenerForDestruction
        {
        public:
            virtual ~ListenerForDestruction() = default;
            ListenerForDestruction() = default;
            virtual void OnDestroy(GraphicsObject const*) {}
        };

        static void SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener);
        static void UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener);

    protected:
        GraphicsObjectType mType;
        std::string mName;

    private:
        // Support for listeners for destruction (LFD).
        static std::mutex msLFDMutex;
        static std::set<std::shared_ptr<ListenerForDestruction>> msLFDSet;
    };
}
