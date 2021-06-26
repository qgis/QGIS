// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Graphics/ProgramDefines.h>
#include <Graphics/DX11/HLSLReflection.h>

namespace gte
{
    class HLSLShaderFactory
    {
    public:
        // Create a shader from an HLSL program in a file.
        static HLSLReflection CreateFromFile(
            std::string const& filename,
            std::string const& entry,
            std::string const& target,
            ProgramDefines const& defines,
            unsigned int compileFlags);

        // Create a shader from an HLSL represented as a string.
        static HLSLReflection CreateFromString(
            std::string const& name,
            std::string const& source,
            std::string const& entry,
            std::string const& target,
            ProgramDefines const& defines,
            unsigned int compileFlags);

        // Create a shader from an HLSL bytecode blob.
        static HLSLReflection CreateFromBytecode(
            std::string const& name,
            std::string const& entry,
            std::string const& target,
            size_t numBytes,
            unsigned char const* bytecode);

    private:
        // Common code for CreateFromFile and CreateFromString.
        static HLSLReflection CompileAndReflect(
            std::string const& name,
            std::string const& source,
            std::string const& entry,
            std::string const& target,
            ProgramDefines const& defines,
            unsigned int compileFlags);

        // Wrapper for the D3DCompile call.
        static ID3DBlob* CompileShader(
            std::string const& name,
            std::string const& source,
            std::string const& entry,
            std::string const& target,
            unsigned int compileFlags,
            ProgramDefines const& defines);

        // Support for shader reflection to obtain information about the HLSL
        // program.
        static bool ReflectShader(
            std::string const& name,
            std::string const& entry,
            std::string const& target,
            ID3DBlob* compiledCode,
            HLSLReflection& shader);

        static bool GetDescription(ID3DShaderReflection* reflector, HLSLReflection& shader);
        static bool GetInputs(ID3DShaderReflection* reflector, HLSLReflection& shader);
        static bool GetOutputs(ID3DShaderReflection* reflector, HLSLReflection& shader);
        static bool GetCBuffers(ID3DShaderReflection* reflector, HLSLReflection& shader);
        static bool GetBoundResources(ID3DShaderReflection* reflector, HLSLReflection& shader);

        static bool GetVariables(ID3DShaderReflectionConstantBuffer* cbuffer,
            unsigned int numVariables, std::vector<HLSLBaseBuffer::Member>& members);

        static bool GetTypes(ID3DShaderReflectionType* rtype,
            unsigned int numMembers, HLSLShaderType& stype);

        static bool IsTextureArray(D3D_SRV_DIMENSION dim);
    };
}
