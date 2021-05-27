// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/GL45/GL45InputLayoutManager.h>

namespace gte
{
    class GL45GraphicsObject;
    class GL45DrawTarget;

    class GL45Engine : public GraphicsEngine
    {
    public:
        // Abstract base class for platform-specific OpenGL engines.
        virtual ~GL45Engine() = default;
        GL45Engine();

        // Currently, OpenGL 4.5 or later is required for compute shaders.
        // Because of deferred construction via Initialize(...), the
        // requirements are not known until that function is called.
        // TODO: Redesign the OpenGL system to allow for earlier versions of
        // OpenGL if the sample application does not require OpenGL 4.5.  This
        // is akin to setting the parameters.featureLevel for Direct3D 11
        // applications.
        inline bool MeetsRequirements() const
        {
            return mMeetsRequirements;
        }

        // Allow the user to switch between OpenGL contexts when there are
        // multiple instances of GL4Engine in an application.
        virtual bool IsActive() const = 0;
        virtual void MakeActive() = 0;

    protected:
        // Helpers for construction and destruction.
        virtual bool Initialize(int requiredMajor, int requiredMinor, bool useDepth24Stencil8, bool saveDriverInfo);
        void Terminate();
        void CreateDefaultFont();
        void DestroyDefaultFont();

        // GTEngine GL45 requires OpenGL 4.5 or later for compute shaders and
        // for CPU-side OpenGL API calls.
        int mMajor, mMinor;
        bool mMeetsRequirements;

    private:
        // Support for drawing.
        uint64_t DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer);

        // Support for enabling and disabling resources used by shaders.
        bool EnableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program);
        void DisableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program);
        void Enable(Shader const* shader, GLuint program);
        void Disable(Shader const* shader, GLuint program);
        void EnableCBuffers(Shader const* shader, GLuint program);
        void DisableCBuffers(Shader const* shader, GLuint program);
        void EnableTBuffers(Shader const* shader, GLuint program);
        void DisableTBuffers(Shader const* shader, GLuint program);
        void EnableSBuffers(Shader const* shader, GLuint program);
        void DisableSBuffers(Shader const* shader, GLuint program);
        void EnableRBuffers(Shader const* shader, GLuint program);
        void DisableRBuffers(Shader const* shader, GLuint program);
        void EnableTextures(Shader const* shader, GLuint program);
        void DisableTextures(Shader const* shader, GLuint program);
        void EnableTextureArrays(Shader const* shader, GLuint program);
        void DisableTextureArrays(Shader const* shader, GLuint program);
        void EnableSamplers(Shader const* shader, GLuint program);
        void DisableSamplers(Shader const* shader, GLuint program);

        // A front-end object (hidden from the user) is created for each
        // atomic counter buffer object declared in use for a shader that is
        // executed.  After execution, these objects are left for use the
        // next time.  They are only destroyed to create new ones when a
        // larger buffer is required, but the buffer size never becomes
        // smaller.  A RawBuffer type is used here because it is by
        // definition 4-bytes per element where 4 bytes is the size for each
        // atomic_uint counter.
        std::vector<std::shared_ptr<RawBuffer>> mAtomicCounterRawBuffers;

        // Keep track of available texture sampler/image units and
        // uniform/shaderstorage buffer units.  If unit is in use, then link
        // count is positive and program+index bound to that unit will be
        // stored.
        class ProgramIndexUnitMap
        {
        public:
            ~ProgramIndexUnitMap() = default;
            ProgramIndexUnitMap() = default;

            int AcquireUnit(GLint program, GLint index);
            int GetUnit(GLint program, GLint index) const;
            void ReleaseUnit(unsigned index);
            unsigned GetUnitLinkCount(unsigned unit) const;
            bool GetUnitProgramIndex(unsigned unit, GLint& program, GLint& index) const;

        private:
            struct LinkInfo
            {
                unsigned linkCount;
                GLint program;
                GLint index;
            };

            std::vector<LinkInfo> mLinkMap;
        };

        ProgramIndexUnitMap mTextureSamplerUnitMap;
        ProgramIndexUnitMap mTextureImageUnitMap;
        ProgramIndexUnitMap mUniformUnitMap;
        ProgramIndexUnitMap mShaderStorageUnitMap;


        // Overrides from GraphicsEngine.
    public:
        // Viewport management.  The measurements are in window coordinates.
        // The origin of the window is (x,y), the window width is w, and the
        // window height is h.  The depth range for the view volume is
        // [zmin,zmax].  The DirectX viewport is left-handed with origin the
        // upper-left corner of the window, the x-axis is directed rightward,
        // the y-axis is directed downward, and the depth range is a subset of
        // [0,1].  The OpenGL viewport is right-handed with origin the
        // lower-left corner of the window, the x-axis is directed rightward,
        // the y-axis is directed upward, and the depth range is a subset of
        // [-1,1].
        virtual void SetViewport(int x, int y, int w, int h) override;
        virtual void GetViewport(int& x, int& y, int& w, int& h) const override;
        virtual void SetDepthRange(float zmin, float zmax) override;
        virtual void GetDepthRange(float& zmin, float& zmax) const override;

        // The function returns 'true' when the depth range is [0,1] (DirectX)
        // or 'false' when the depth range is [-1,1] (OpenGL).
        virtual bool HasDepthRange01() const
        {
            return false;
        }

        // Append the extension of the shader file to 'name' (.hlsl for DirectX,
        // .glsl for OpenGL).
        virtual std::string GetShaderName(std::string const& name) const
        {
            return name + ".glsl";
        }

        // Window resizing.
        virtual bool Resize(unsigned int w, unsigned int h) override;

        // Support for clearing the color, depth, and stencil back buffers.
        virtual void ClearColorBuffer() override;
        virtual void ClearDepthBuffer() override;
        virtual void ClearStencilBuffer() override;
        virtual void ClearBuffers() override;

        // Global drawing state.  The default states are listed in the headers
        // BlendState.h, DepthStencil.h and RasterizerState.h.
        virtual void SetBlendState(std::shared_ptr<BlendState> const& state) override;
        virtual void SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state) override;
        virtual void SetRasterizerState(std::shared_ptr<RasterizerState> const& state) override;

        // Support for drawing to offscreen memory (i.e. not to the back
        // buffer).  The DrawTarget object encapsulates render targets (color
        // information) and depth-stencil target.
        virtual void Enable(std::shared_ptr<DrawTarget> const& target) override;
        virtual void Disable(std::shared_ptr<DrawTarget> const& target) override;

        // Support for copying from CPU to GPU via mapped memory.
        virtual bool Update(std::shared_ptr<Buffer> const& buffer) override;
        virtual bool Update(std::shared_ptr<TextureSingle> const& texture) override;
        virtual bool Update(std::shared_ptr<TextureSingle> const& texture, unsigned int level) override;
        virtual bool Update(std::shared_ptr<TextureArray> const& textureArray) override;
        virtual bool Update(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) override;

        // Support for copying from CPU to GPU via staging memory.
        virtual bool CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer) override;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture) override;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) override;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray) override;
        virtual bool CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) override;

        // Support for copying from GPU to CPU via staging memory.
        virtual bool CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer) override;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture) override;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level) override;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray) override;
        virtual bool CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level) override;

        // Support for copying from GPU to GPU directly.  TODO: We will
        // improve on the feature set for such copies later.  For now, the
        // restrictions are that the resources are different, of the same
        // type, have identical dimensions, and have compatible formats (if
        // of texture type).
        virtual void CopyGpuToGpu(
            std::shared_ptr<Buffer> const& buffer0,
            std::shared_ptr<Buffer> const& buffer1) override;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureSingle> const& texture0,
            std::shared_ptr<TextureSingle> const& texture1) override;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureSingle> const& texture0,
            std::shared_ptr<TextureSingle> const& texture1,
            unsigned int level) override;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureArray> const& textureArray0,
            std::shared_ptr<TextureArray> const& textureArray1) override;

        virtual void CopyGpuToGpu(
            std::shared_ptr<TextureArray> const& textureArray0,
            std::shared_ptr<TextureArray> const& textureArray1,
            unsigned int item, unsigned int level) override;

        // Counted buffer management.  GetNumActiveElements stores the result
        // in 'buffer'.
        virtual bool GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer) override;

        // Execute the compute program.  If you want the CPU to stall to wait
        // for the results, call WaitForFinish() immediately after
        // Execute(...).  However, you can synchronize CPU and GPU activity by
        // calling WaitForFinish() at some later time, the goal being not to
        // stall the CPU before obtaining the GPU results.
        virtual bool BindProgram(std::shared_ptr<ComputeProgram> const& program) override;
        virtual void Execute(std::shared_ptr<ComputeProgram> const& program,
            unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups) override;

        // Have the CPU wait until the GPU finishes its current command
        // buffer.
        virtual void WaitForFinish() override;

        // Flush the command buffer.
        virtual void Flush() override;

    private:
        // Support for drawing.  If occlusion queries are enabled, the return
        // value is the number of samples that passed the depth and stencil
        // tests, effectively the number of pixels drawn.  If occlusion queries
        // are disabled, the function returns 0.
        virtual uint64_t DrawPrimitive(
            std::shared_ptr<VertexBuffer> const& vbuffer,
            std::shared_ptr<IndexBuffer> const& ibuffer,
            std::shared_ptr<VisualEffect> const& effect) override;
    };
}
