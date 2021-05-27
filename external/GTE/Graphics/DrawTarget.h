// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/TextureDS.h>
#include <Graphics/TextureRT.h>

namespace gte
{
    class DrawTarget
    {
    public:
        // Construction and destruction.  The number of supported targets
        // depends on the graphics hardware and drivers.  The number of render
        // targets must be at least 1.  Set 'hasRTData' to 'true' when you
        // want system memory data for the color textures.  Set 'dsFormat' to
        // a valid format when you want depth-stencil; these are
        //   DF_D32_FLOAT_S8X24_UINT
        //   DF_D32_FLOAT
        //   DF_D24_UNORM_S8_UINT
        //   DF_D16_UNORM
        // Set 'dsFormat' to DF_UNKNOWN when you do not want a depth-stencil.
        // Set 'hasDSData' to 'true' iff you want system memory data for the
        // depth-stencil.
        virtual ~DrawTarget();

        DrawTarget(unsigned int numRenderTargets, DFType rtFormat,
            unsigned int width, unsigned int height, bool hasRTMipmaps = false,
            bool createRTStorage = true, DFType dsFormat = DF_UNKNOWN,
            bool createDSStorage = false);

        // Member access.
        inline unsigned int GetNumTargets() const
        {
            return static_cast<unsigned int>(mRTTextures.size());
        }

        DFType GetRTFormat() const;
        unsigned int GetWidth() const;
        unsigned int GetHeight() const;
        bool HasRTMipmaps() const;
        DFType GetDSFormat() const;
        std::shared_ptr<TextureRT> const GetRTTexture(unsigned int i) const;

        inline std::shared_ptr<TextureDS> const GetDSTexture() const
        {
            return mDSTexture;
        }

        // Request that the GPU compute mipmap levels when the base-level
        // render target is written.  The AutogenerateMipmaps call should be
        // made before binding the texture to the engine.  If the texture does
        // not have mipmaps, the AutogenerateMipmaps call will not set
        // mAutogenerateMipmaps to true.
        void AutogenerateRTMipmaps();
        bool WantAutogenerateRTMipmaps() const;

        // Listeners subscribe to receive notification when a DrawTarget is
        // about to be destroyed.  The intended use is for the graphics-API
        // specific engines to destroy corresponding graphics-API draw
        // target objects.
        class ListenerForDestruction
        {
        public:
            virtual ~ListenerForDestruction() = default;
            ListenerForDestruction() = default;
            virtual void OnDestroy(DrawTarget const*) {}
        };

        static void SubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener);
        static void UnsubscribeForDestruction(std::shared_ptr<ListenerForDestruction> const& listener);

    protected:
        std::vector<std::shared_ptr<TextureRT>> mRTTextures;
        std::shared_ptr<TextureDS> mDSTexture;

    private:
        // Support for listeners for destruction (LFD).
        static std::mutex msLFDMutex;
        static std::set<std::shared_ptr<ListenerForDestruction>> msLFDSet;
    };
}
