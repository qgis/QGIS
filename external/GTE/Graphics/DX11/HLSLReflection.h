// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/DX11/HLSLByteAddressBuffer.h>
#include <Graphics/DX11/HLSLConstantBuffer.h>
#include <Graphics/DX11/HLSLParameter.h>
#include <Graphics/DX11/HLSLResourceBindInfo.h>
#include <Graphics/DX11/HLSLSamplerState.h>
#include <Graphics/DX11/HLSLStructuredBuffer.h>
#include <Graphics/DX11/HLSLTexture.h>
#include <Graphics/DX11/HLSLTextureArray.h>
#include <Graphics/DX11/HLSLTextureBuffer.h>

namespace gte
{
    class HLSLReflection
    {
    public:
        struct Description
        {
            struct InstructionCount
            {
                unsigned int numInstructions;
                unsigned int numTemporaryRegisters;
                unsigned int numTemporaryArrays;
                unsigned int numDefines;
                unsigned int numDeclarations;
                unsigned int numTextureNormal;
                unsigned int numTextureLoad;
                unsigned int numTextureComparison;
                unsigned int numTextureBias;
                unsigned int numTextureGradient;
                unsigned int numFloatArithmetic;
                unsigned int numSIntArithmetic;
                unsigned int numUIntArithmetic;
                unsigned int numStaticFlowControl;
                unsigned int numDynamicFlowControl;
                unsigned int numMacro;
                unsigned int numArray;
            };


            struct GSParameters
            {
                unsigned int numCutInstructions;
                unsigned int numEmitInstructions;
                D3D_PRIMITIVE inputPrimitive;
                D3D_PRIMITIVE_TOPOLOGY outputTopology;
                unsigned int maxOutputVertices;
            };

            struct TSParameters
            {
                unsigned int numPatchConstants;
                unsigned int numGSInstances;
                unsigned int numControlPoints;
                D3D_PRIMITIVE inputPrimitive;
                D3D_TESSELLATOR_OUTPUT_PRIMITIVE outputPrimitive;
                D3D_TESSELLATOR_PARTITIONING partitioning;
                D3D_TESSELLATOR_DOMAIN domain;
            };

            struct CSParameters
            {
                unsigned int numBarrierInstructions;
                unsigned int numInterlockedInstructions;
                unsigned int numTextureStoreInstructions;
            };

            std::string creator;
            D3D_SHADER_VERSION_TYPE shaderType;
            unsigned int majorVersion;
            unsigned int minorVersion;
            unsigned int flags;
            unsigned int numConstantBuffers;
            unsigned int numBoundResources;
            unsigned int numInputParameters;
            unsigned int numOutputParameters;
            InstructionCount instructions;
            GSParameters gs;
            TSParameters ts;
            CSParameters cs;
        };

        // Construction.
        HLSLReflection();

        // Test whether the shader was constructed properly.  The function tests
        // solely whether the name, entry, and target are nonempty strings and
        // that the compiled code array is nonempty; this is the common case when
        // HLSLShaderFactory is used to create the shader.
        bool IsValid() const;

        // Deferred construction for shader reflection.  These functions are
        // intended to be write-once.
        void SetDescription(D3D_SHADER_DESC const& desc);

        inline void SetName(std::string const& name)
        {
            mName = name;
        }

        inline void SetEntry(std::string const& entry)
        {
            mEntry = entry;
        }

        inline void SetTarget(std::string const& target)
        {
            mTarget = target;
        }

        inline void InsertInput(HLSLParameter const& parameter)
        {
            mInputs.push_back(parameter);
        }

        inline void InsertOutput(HLSLParameter const& parameter)
        {
            mOutputs.push_back(parameter);
        }

        inline void Insert(HLSLConstantBuffer const& cbuffer)
        {
            mCBuffers.push_back(cbuffer);
        }

        inline void Insert(HLSLTextureBuffer const& tbuffer)
        {
            mTBuffers.push_back(tbuffer);
        }

        inline void Insert(HLSLStructuredBuffer const& sbuffer)
        {
            mSBuffers.push_back(sbuffer);
        }

        inline void Insert(HLSLByteAddressBuffer const& rbuffer)
        {
            mRBuffers.push_back(rbuffer);
        }

        inline void Insert(HLSLTexture const& texture)
        {
            mTextures.push_back(texture);
        }

        inline void Insert(HLSLTextureArray const& textureArray)
        {
            mTextureArrays.push_back(textureArray);
        }

        inline void Insert(HLSLSamplerState const& samplerState)
        {
            mSamplerStates.push_back(samplerState);
        }

        inline void Insert(HLSLResourceBindInfo const& rbinfo)
        {
            mRBInfos.push_back(rbinfo);
        }

        void SetCompiledCode(size_t numBytes, void const* buffer);

        // Member access.
        inline HLSLReflection::Description const& GetDescription() const
        {
            return mDesc;
        }

        inline std::string const& GetName() const
        {
            return mName;
        }

        inline std::string const& GetEntry() const
        {
            return mEntry;
        }

        inline std::string const& GetTarget() const
        {
            return mTarget;
        }

        int GetShaderTypeIndex() const;

        inline std::vector<HLSLParameter> const& GetInputs() const
        {
            return mInputs;
        }

        inline std::vector<HLSLParameter> const& GetOutputs() const
        {
            return mOutputs;
        }

        inline std::vector<HLSLConstantBuffer> const& GetCBuffers() const
        {
            return mCBuffers;
        }

        inline std::vector<HLSLTextureBuffer> const& GetTBuffers() const
        {
            return mTBuffers;
        }

        inline std::vector<HLSLStructuredBuffer> const& GetSBuffers() const
        {
            return mSBuffers;
        }

        inline std::vector<HLSLByteAddressBuffer> const& GetRBuffers() const
        {
            return mRBuffers;
        }

        inline std::vector<HLSLTexture> const& GetTextures() const
        {
            return mTextures;
        }

        inline std::vector<HLSLTextureArray> const& GetTextureArrays() const
        {
            return mTextureArrays;
        }

        inline std::vector<HLSLSamplerState> const& GetSamplerStates() const
        {
            return mSamplerStates;
        }

        inline std::vector<HLSLResourceBindInfo> const& GetResourceBindInfos() const
        {
            return mRBInfos;
        }

        inline std::vector<unsigned char> const& GetCompiledCode() const
        {
            return mCompiledCode;
        }

        // Compute shaders only.
        void SetNumThreads(unsigned int numXThreads, unsigned int numYThreads, unsigned int numZThreads);

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

        // Print to a text file for human readability.
        void Print(std::ofstream& output) const;

    private:
        Description mDesc;
        std::string mName;
        std::string mEntry;
        std::string mTarget;
        std::vector<HLSLParameter> mInputs;
        std::vector<HLSLParameter> mOutputs;
        std::vector<HLSLConstantBuffer> mCBuffers;
        std::vector<HLSLTextureBuffer> mTBuffers;
        std::vector<HLSLStructuredBuffer> mSBuffers;
        std::vector<HLSLByteAddressBuffer> mRBuffers;
        std::vector<HLSLTexture> mTextures;
        std::vector<HLSLTextureArray> mTextureArrays;
        std::vector<HLSLSamplerState> mSamplerStates;
        std::vector<HLSLResourceBindInfo> mRBInfos;
        std::vector<unsigned char> mCompiledCode;
        unsigned int mNumXThreads;
        unsigned int mNumYThreads;
        unsigned int mNumZThreads;

        // Support for Print.
        static std::string const msShaderType[];
        static std::string const msCompileFlags[];
        static std::string const msPrimitive[];
        static std::string const msPrimitiveTopology[];
        static std::string const msOutputPrimitive[];
        static std::string const msPartitioning[];
        static std::string const msDomain[];
    };
}
