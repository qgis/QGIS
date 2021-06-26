// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.12

#pragma once

#include <Graphics/GraphicsEngine.h>
#include <Graphics/DX11/DX11InputLayoutManager.h>
#include <Graphics/DX11/DX11PerformanceCounter.h>
#include <Graphics/DX11/DXGIOutput.h>

namespace gte
{
    class DX11DrawTarget;
    class DX11GeometryShader;
    class DX11GraphicsObject;
    class DX11PixelShader;
    class DX11Shader;
    class DX11Texture2;
    class DX11VertexShader;

    class DX11Engine : public GraphicsEngine
    {
    public:
        // Construction and destruction.
        virtual ~DX11Engine();

        // Constructors for computing.  The first constructor uses the default
        // adapter and tries for DX11.1 hardware acceleration (the maximum
        // feature level) without debugging support; it is equivalent to the
        // second constructor call:
        //   DX11Engine(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0);
        //
        DX11Engine();
        DX11Engine(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
            HMODULE softwareModule, UINT flags);

        // Constructors for graphics (automatically get computing
        // capabilities).  The first constructor uses the default adapter and
        // tries for DX11.0 hardware acceleration (the maximum feature level)
        // without debugging support; it is equivalent to the second
        // constructor call:
        //   DX11Engine(nullptr, handle, xSize, ySize,
        //       D3D_DRIVER_TYPE_HARDWARE, nullptr, 0);
        //
        DX11Engine(HWND handle, UINT xSize, UINT ySize, bool useDepth24Stencil8 = true);
        DX11Engine(IDXGIAdapter* adapter, HWND handle, UINT xSize, UINT ySize,
            bool useDepth24Stencil8, D3D_DRIVER_TYPE driverType, HMODULE softwareModule,
            UINT flags);

        // Access to members that correspond to constructor inputs.
        inline IDXGIAdapter* GetAdapter() const
        {
            return mAdapter;
        }

        inline D3D_DRIVER_TYPE GetDriverType() const
        {
            return mDriverType;
        }

        inline HMODULE GetSoftwareModule() const
        {
            return mSoftwareModule;
        }

        inline UINT GetFlags() const
        {
            return mFlags;
        }

        // Give access to the device and immediate context in case you want
        // to use DX11-specific graphics in an application.  You must typecast
        // the std::shared_ptr<GraphicsEngine> object, mEngine, object in the
        // application layer to std::shared_ptr<DX11Engine>. 
        inline ID3D11Device* GetDevice() const
        {
            return mDevice;
        }

        inline ID3D11DeviceContext* GetImmediate() const
        {
            return mImmediate;
        }

        // Return the feature level of the created device.  This will be a
        // feature less than DX11.1 when your graphics driver does not
        // support DX11.1.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return mFeatureLevel;
        }

        // Support for sharing textures by devices.  If the 'texture' is bound
        // to the 'engine', then it is shared by 'this'.  If it is not bound
        // to 'engine', this function behaves just like 'Bind'.  In both
        // cases, a new DX11Texture2 wrapper is returned that is associated
        // with 'this'.  You may unbind as desired.  If the shared texture is
        // updated on one device, Flush() must be called on that device.
        DX11GraphicsObject* Share(std::shared_ptr<Texture2> const& texture, DX11Engine* engine);

        // Mapped copying from CPU to GPU.
        D3D11_MAPPED_SUBRESOURCE MapForWrite(std::shared_ptr<Resource> const& resource, unsigned int sri);
        void Unmap(std::shared_ptr<Resource> const& resource, unsigned int sri);

        // Support for toggling between window and fullscreen modes.  The
        // return value is 'true' iff the operation succeeded.
        bool IsFullscreen(DXGIOutput const& output) const;
        bool SetFullscreen(DXGIOutput const& output, bool fullscreen);
        void ExitFullscreen();

        // Support for GPU timing. This is a coarse-level measurement system.
        // Each graphics card manufacturer provides more accurate measurements
        // and more performance counters than just simple timing.
        void BeginTimer(DX11PerformanceCounter& counter);
        void EndTimer(DX11PerformanceCounter& counter);

    private:
        // Helpers for construction and destruction.
        void Initialize(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType,
            HMODULE softwareModule, UINT flags, bool useDepth24Stencil8);
        bool CreateDevice();
        bool CreateBestMatchingDevice();
        bool CreateSwapChain(HWND handle, UINT xSize, UINT ySize);
        bool CreateBackBuffer(UINT xSize, UINT ySize);
        void CreateDefaultObjects();
        void DestroyDefaultObjects();
        bool DestroyDevice();
        bool DestroySwapChain();
        bool DestroyBackBuffer();

        // Support for drawing.
        uint64_t DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer);
        ID3D11Query* BeginOcclusionQuery();
        uint64_t EndOcclusionQuery(ID3D11Query* occlusionQuery);

        // Support for enabling and disabling resources used by shaders.
        bool EnableShaders(std::shared_ptr<VisualEffect> const& effect, DX11VertexShader*& dxVShader, DX11GeometryShader*& dxGShader, DX11PixelShader*& dxPShader);
        void DisableShaders(std::shared_ptr<VisualEffect> const& effect, DX11VertexShader* dxVShader, DX11GeometryShader* dxGShader, DX11PixelShader* dxPShader);
        void Enable(Shader const* shader, DX11Shader* dxShader);
        void Disable(Shader const* shader, DX11Shader* dxShader);
        void EnableCBuffers(Shader const* shader, DX11Shader* dxShader);
        void DisableCBuffers(Shader const* shader, DX11Shader* dxShader);
        void EnableTBuffers(Shader const* shader, DX11Shader* dxShader);
        void DisableTBuffers(Shader const* shader, DX11Shader* dxShader);
        void EnableSBuffers(Shader const* shader, DX11Shader* dxShader);
        void DisableSBuffers(Shader const* shader, DX11Shader* dxShader);
        void EnableRBuffers(Shader const* shader, DX11Shader* dxShader);
        void DisableRBuffers(Shader const* shader, DX11Shader* dxShader);
        void EnableTextures(Shader const* shader, DX11Shader* dxShader);
        void DisableTextures(Shader const* shader, DX11Shader* dxShader);
        void EnableTextureArrays(Shader const* shader, DX11Shader* dxShader);
        void DisableTextureArrays(Shader const* shader, DX11Shader* dxShader);
        void EnableSamplers(Shader const* shader, DX11Shader* dxShader);
        void DisableSamplers(Shader const* shader, DX11Shader* dxShader);

        // Inputs to the constructors.  If mUseDepth24Stencil8 is 'true', the
        // back buffer has a 24-bit depth and 8-bit stencil buffer.  If the
        // value is 'false', the back buffer has a 32-bit depth buffer (no
        // stencil information).
        IDXGIAdapter* mAdapter;
        D3D_DRIVER_TYPE mDriverType;
        HMODULE mSoftwareModule;
        UINT mFlags;
        bool mUseDepth24Stencil8;

        // Objects created by the constructors. If the constructors taking an
        // HWND parameter are called, mIsGraphicsDevice is true. Otherwise,
        // the constructors are called for computing and mIsGraphicsDevice is
        // false.
        ID3D11Device* mDevice;
        ID3D11DeviceContext* mImmediate;
        D3D_FEATURE_LEVEL mFeatureLevel;
        bool mIsGraphicsDevice;

        // Objects created by the constructors for graphics engines.
        IDXGISwapChain* mSwapChain;
        ID3D11Texture2D* mColorBuffer;
        ID3D11RenderTargetView* mColorView;
        ID3D11Texture2D* mDepthStencilBuffer;
        ID3D11DepthStencilView* mDepthStencilView;
        D3D11_VIEWPORT mViewport;

        // Support for draw target enabling and disabling.
        unsigned int mNumActiveRTs;
        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> mActiveRT;
        ID3D11DepthStencilView* mActiveDS;
        D3D11_VIEWPORT mSaveViewport;
        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> mSaveRT;
        ID3D11DepthStencilView* mSaveDS;

        // For synchronization (wait for the GPU command buffer to be fully
        // executed).  The query is created on-demand (the first time
        // WaitForFinish is called).
        ID3D11Query* mWaitQuery;

        // Keep track of whether the window is fullscreen or normal mode.  As
        // recommended by MSDN documentation, we do not use
        // swapChainDesc.Windowed to control this; rather, we create a
        // windowed-mode device and allow the programmer to use
        // SetFullscreenState to toggle between windowed and fullscreen.
        std::map<std::wstring, bool> mFullscreenState;


        // Overrides from BaseEngine.
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
            return true;
        }

        // Append the extension of the shader file to 'name' (.hlsl for DirectX,
        // .glsl for OpenGL).
        virtual std::string GetShaderName(std::string const& name) const
        {
            return name + ".hlsl";
        }

        // Window resizing.
        virtual bool Resize(unsigned int w, unsigned int h) override;

        // Support for clearing the color, depth, and stencil back buffers.
        virtual void ClearColorBuffer() override;
        virtual void ClearDepthBuffer() override;
        virtual void ClearStencilBuffer() override;
        virtual void ClearBuffers() override;
        virtual void DisplayColorBuffer(unsigned int syncInterval) override;

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

        // Allow creation of a DX11Texture2 object from known ID3D11*
        // interfaces.
        DX11Texture2* BindTexture(std::shared_ptr<Texture2> const& texture, ID3D11Texture2D* dxTexture, ID3D11ShaderResourceView* dxSRView);

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
        virtual void CopyGpuToGpu(std::shared_ptr<Buffer> const& buffer0, std::shared_ptr<Buffer> const& buffer1) override;
        virtual void CopyGpuToGpu(std::shared_ptr<TextureSingle> const& texture0, std::shared_ptr<TextureSingle> const& texture1) override;
        virtual void CopyGpuToGpu(std::shared_ptr<TextureSingle> const& texture0, std::shared_ptr<TextureSingle> const& texture1, unsigned int level) override;
        virtual void CopyGpuToGpu(std::shared_ptr<TextureArray> const& textureArray0, std::shared_ptr<TextureArray> const& textureArray1) override;
        virtual void CopyGpuToGpu(std::shared_ptr<TextureArray> const& textureArray0, std::shared_ptr<TextureArray> const& textureArray1, unsigned int item, unsigned int level) override;

        // Counted buffer management.  GetNumActiveElements stores the result
        // in 'buffer'.
        virtual bool GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer) override;

        // Execute the compute program.  If you want the CPU to stall to wait
        // for the results, call WaitForFinish() immediately after
        // Execute(...).  However, you can synchronize CPU and GPU activity by
        // calling WaitForFinish() at some later time, the goal being not to
        // stall the CPU before obtaining the GPU results.
        virtual bool BindProgram(std::shared_ptr<ComputeProgram> const& program) override;
        virtual void Execute(std::shared_ptr<ComputeProgram> const& program, unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups) override;

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

    public:
        // If the input texture does not match the back-buffer format and
        // dimensions, it will be recreated.
        //
        // TODO: This is specific to the DirectX 11 engine. Add the same
        // support to the OpenGL engine.
        void CopyBackBuffer(std::shared_ptr<Texture2>& texture);

    private:
        ID3D11Texture2D* mBackBufferStaging;
    };
}
