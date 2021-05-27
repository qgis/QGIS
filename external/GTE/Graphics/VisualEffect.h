// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Matrix4x4.h>
#include <Graphics/VisualProgram.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/Texture.h>
#include <Graphics/TextureArray.h>

namespace gte
{
    class VisualEffect
    {
    public:
        // Construction and destruction.
        virtual ~VisualEffect() = default;
        VisualEffect(std::shared_ptr<VisualProgram> const& program);

        // Member access.
        inline std::shared_ptr<VisualProgram> const& GetProgram() const
        {
            return mProgram;
        }

        inline std::shared_ptr<Shader> const& GetVertexShader() const
        {
            return mProgram->GetVertexShader();
        }

        inline std::shared_ptr<Shader> const& GetPixelShader() const
        {
            return mProgram->GetPixelShader();
        }

        inline std::shared_ptr<Shader> const& GetGeometryShader() const
        {
            return mProgram->GetGeometryShader();
        }

        // For convenience, provide a projection-view-world constant buffer
        // that an effect can use if so desired.
        virtual void SetPVWMatrixConstant(std::shared_ptr<ConstantBuffer> const& buffer);

        inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const
        {
            return mPVWMatrixConstant;
        }

        inline void SetPVWMatrix(Matrix4x4<float> const& pvwMatrix)
        {
            *mPVWMatrixConstant->Get<Matrix4x4<float>>() = pvwMatrix;
        }

        inline Matrix4x4<float> const& GetPVWMatrix() const
        {
            return *mPVWMatrixConstant->Get<Matrix4x4<float>>();
        }

    protected:
        // For derived classes to defer construction because they want to
        // create programs via a factory.
        VisualEffect();

        std::shared_ptr<VisualProgram> mProgram;
        BufferUpdater mBufferUpdater;
        TextureUpdater mTextureUpdater;
        TextureArrayUpdater mTextureArrayUpdater;

        // The constant buffer that stores the 4x4 projection-view-world
        // transformation for the Visual object to which this effect is
        // attached.
        std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    };
}
