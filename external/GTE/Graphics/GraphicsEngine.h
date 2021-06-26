// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/BaseEngine.h>
#include <Graphics/GEDrawTarget.h>
#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/GEObject.h>
#include <Graphics/DrawTarget.h>
#include <Graphics/FontArialW400H18.h>
#include <Graphics/Visual.h>
#include <Mathematics/ThreadSafeMap.h>
#include <array>

// TODO: It appears that BaseEngine was separated out from GraphicsEngine
// in order for the listener system of GraphicsEngine to function properly
// by destroying objects at the correct time.  Redesign the graphics system
// so that there is a single base class GraphicsEngine (eliminate BaseEngine).

namespace gte
{
    class GraphicsEngine : public BaseEngine
    {
    public:
        // Abstract base class.
        virtual ~GraphicsEngine() = default;
    protected:
        GraphicsEngine();

        // Disallow copying and assignment.
        GraphicsEngine(GraphicsEngine const&) = delete;
        GraphicsEngine& operator=(GraphicsEngine const&) = delete;

    public:
        // Support for clearing the color, depth, and stencil back buffers.
        virtual void ClearColorBuffer() = 0;
        virtual void ClearDepthBuffer() = 0;
        virtual void ClearStencilBuffer() = 0;
        virtual void ClearBuffers() = 0;

        // Support for bitmapped fonts used in text rendering.  The default
        // font is Arial (height 18, no italics, no bold).
        virtual void SetFont(std::shared_ptr<Font> const& font) override;

        // Support for drawing.  If occlusion queries are enabled, the return
        // values are the number of samples that passed the depth and stencil
        // tests, effectively the number of pixels drawn.  If occlusion
        // queries are disabled, the functions return 0.

        // Draw geometric primitives.
        uint64_t Draw(Visual* visual);
        uint64_t Draw(std::vector<Visual*> const& visuals);
        uint64_t Draw(std::shared_ptr<Visual> const& visual);
        uint64_t Draw(std::vector<std::shared_ptr<Visual>> const& visuals);

        // Draw 2D text.
        uint64_t Draw(int x, int y, std::array<float, 4> const& color, std::string const& message);

        // Draw a 2D rectangular overlay.  This is useful for adding buttons,
        // controls, thumbnails, and other GUI objects to an application
        // window.
        virtual uint64_t Draw(std::shared_ptr<OverlayEffect> const& overlay) override;

        // Support for occlusion queries.  When enabled, Draw functions return
        // the number of samples that passed the depth and stencil tests,
        // effectively the number of pixels drawn.  The default value is
        // 'false'.
        inline void AllowOcclusionQuery(bool allow)
        {
            mAllowOcclusionQuery = allow;
        }

        // Support for drawing to offscreen memory (i.e. not to the back
        // buffer).  The DrawTarget object encapsulates render targets (color
        // information) and depth-stencil target.
        virtual void Enable(std::shared_ptr<DrawTarget> const& target) = 0;
        virtual void Disable(std::shared_ptr<DrawTarget> const& target) = 0;

        // Graphics object management.  The Bind function creates a graphics
        // API-specific object that corresponds to the input GTEngine object.
        // GraphicsEngine manages this bridge mapping internally.  The Unbind
        // function destroys the graphics API-specific object.  These may be
        // called explicitly, but the engine is designed to create on demand
        // and to destroy on device destruction.
        GEObject* Bind(std::shared_ptr<GraphicsObject> const& object);
        virtual bool BindProgram(std::shared_ptr<ComputeProgram> const& program) = 0;
        GEDrawTarget* Bind(std::shared_ptr<DrawTarget> const& target);
        GEObject* Get(std::shared_ptr<GraphicsObject> const& object) const;
        GEDrawTarget* Get(std::shared_ptr<DrawTarget> const& target) const;

        inline bool Unbind(std::shared_ptr<GraphicsObject> const& object)
        {
            return Unbind(object.get());
        }

        inline bool Unbind(std::shared_ptr<DrawTarget> const& target)
        {
            return Unbind(target.get());
        }

        void GetTotalAllocation(size_t& numBytes, size_t& numObjects) const;

        // Support for copying from CPU to GPU via mapped memory.
        virtual bool Update(std::shared_ptr<Buffer> const& buffer) = 0;
        virtual bool Update(std::shared_ptr<TextureSingle> const& texture) = 0;
        virtual bool Update(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
        virtual bool Update(std::shared_ptr<TextureArray> const& textureArray) = 0;
        virtual bool Update(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

        // Support for copying from CPU to GPU via staging memory.
        virtual bool CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer) = 0;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture) = 0;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray) = 0;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

        // Support for copying from GPU to CPU via staging memory.
        virtual bool CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer) = 0;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture) = 0;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) = 0;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray) = 0;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) = 0;

        // Support for copying from GPU to GPU directly.  TODO: We will
        // improve on the feature set for such copies later.  For now, the
        // restrictions are that the resources are different, of the same
        // type, have identical dimensions, and have compatible formats (if
        // of texture type).
        virtual void CopyGpuToGpu(
            std::shared_ptr<Buffer> const& buffer0,
            std::shared_ptr<Buffer> const& buffer1) = 0;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureSingle> const& texture0,
            std::shared_ptr<TextureSingle> const& texture1) = 0;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureSingle> const& texture0,
            std::shared_ptr<TextureSingle> const& texture1,
            unsigned int level) = 0;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureArray> const& textureArray0,
            std::shared_ptr<TextureArray> const& textureArray1) = 0;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureArray> const& textureArray0,
            std::shared_ptr<TextureArray> const& textureArray1,
            unsigned int item, unsigned int level) = 0;

        // Counted buffer management.  GetNumActiveElements stores the result
        // in 'buffer'.
        virtual bool GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer) = 0;

        // Execute the compute program.  If you want the CPU to stall to wait
        // for the results, call WaitForFinish() immediately after
        // Execute(...).  However, you can synchronize CPU and GPU activity by
        // calling WaitForFinish() at some later time, the goal being not to
        // stall the CPU before obtaining the GPU results.
        virtual void Execute(std::shared_ptr<ComputeProgram> const& program,
            unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups) = 0;

        // Have the CPU wait until the GPU finishes its current command
        // buffer.
        virtual void WaitForFinish() = 0;

        // Flush the command buffer.
        virtual void Flush() = 0;

        // Set the warning to 'true' if you want the DX11Engine destructor to
        // report that the bridge maps are nonempty.  If they are, the
        // application did not destroy GraphicsObject items before the engine
        // was destroyed.  The default values is 'true'.
        inline void WarnOnNonemptyBridges(bool warn)
        {
            mWarnOnNonemptyBridges = warn;
        }

    protected:

        // Helper for destruction.
        virtual void DestroyDefaultGlobalState();

        // Support for drawing.  If occlusion queries are enabled, the return
        // values are the number of samples that passed the depth and stencil
        // tests, effectively the number of pixels drawn.  If occlusion
        // queries are disabled, the functions return 0.
        virtual uint64_t DrawPrimitive(
            std::shared_ptr<VertexBuffer> const& vbuffer,
            std::shared_ptr<IndexBuffer> const& ibuffer,
            std::shared_ptr<VisualEffect> const& effect) = 0;

        // Support for GOListener::OnDestroy and DTListener::OnDestroy,
        // because they are passed raw pointers from resource destructors.
        // These are also used by the Unbind calls whose inputs are
        // std::shared_ptr<T>.
        bool Unbind(GraphicsObject const* object);
        bool Unbind(DrawTarget const* target);


        // Bridge pattern to create graphics API-specific objects that
        // correspond to front-end objects.  The Bind, Get, and Unbind
        // operations act on these maps.
        ThreadSafeMap<GraphicsObject const*, std::shared_ptr<GEObject>> mGOMap;
        ThreadSafeMap<DrawTarget const*, std::shared_ptr<GEDrawTarget>> mDTMap;
        std::unique_ptr<GEInputLayoutManager> mILMap;

        // Creation functions for adding objects to the bridges.  The
        // function pointers are assigned during construction.
        typedef std::shared_ptr<GEObject>(*CreateGEObject)(void*, GraphicsObject const*);
        typedef std::shared_ptr<GEDrawTarget>(*CreateGEDrawTarget)(DrawTarget const*,
            std::vector<GEObject*>&, GEObject*);
        std::array<CreateGEObject, GT_NUM_TYPES> mCreateGEObject;
        CreateGEDrawTarget mCreateGEDrawTarget;
        void* mGEObjectCreator;

        // Track GraphicsObject destruction and delete to-be-destroyed objects
        // from the bridge map.
        class GOListener : public GraphicsObject::ListenerForDestruction
        {
        public:
            virtual ~GOListener() = default;
            GOListener(GraphicsEngine* engine);
            virtual void OnDestroy(GraphicsObject const* object);
        private:
            GraphicsEngine* mEngine;
        };

        std::shared_ptr<GOListener> mGOListener;

        // Track DrawTarget destruction and delete to-be-destroyed objects
        // from the draw target map.
        class DTListener : public DrawTarget::ListenerForDestruction
        {
        public:
            virtual ~DTListener() = default;
            DTListener(GraphicsEngine* engine);
            virtual void OnDestroy(DrawTarget const* target);
        private:
            GraphicsEngine* mEngine;
        };

        std::shared_ptr<DTListener> mDTListener;

        bool mAllowOcclusionQuery;
        bool mWarnOnNonemptyBridges;
    };
}
