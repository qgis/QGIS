// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2020.04.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLShaderFactory.h>
using namespace gte;

HLSLReflection HLSLShaderFactory::CreateFromFile(std::string const& filename,
    std::string const& entry, std::string const& target,
    ProgramDefines const& defines, unsigned int compileFlags)
{
    std::ifstream input(filename);
    if (!input)
    {
        LogError("Cannot open file " + filename);
    }

    std::string source = "";
    while (!input.eof())
    {
        std::string line;
        getline(input, line);
        source += line + "\n";
    }
    input.close();

    return CompileAndReflect(filename, source, entry, target, defines, compileFlags);
}

HLSLReflection HLSLShaderFactory::CreateFromString(std::string const& name,
    std::string const& source, std::string const& entry, std::string const& target,
    ProgramDefines const& defines, unsigned int compileFlags)
{
    return CompileAndReflect(name, source, entry, target, defines, compileFlags);
}

HLSLReflection HLSLShaderFactory::CreateFromBytecode(
    std::string const& name,
    std::string const& entry,
    std::string const& target,
    size_t numBytes,
    unsigned char const* bytecode)
{
    ID3DBlob* compiledCode = nullptr;
    DX11Log(D3DCreateBlob(numBytes, &compiledCode));

    std::memcpy(compiledCode->GetBufferPointer(), bytecode, numBytes);

    HLSLReflection shader;
    if (!ReflectShader(name, entry, target, compiledCode, shader))
    {
        // Errors are recorded to a logfile by ReflectShader.
        shader = HLSLReflection();
    }
    DX11::SafeRelease(compiledCode);
    return shader;
}

HLSLReflection HLSLShaderFactory::CompileAndReflect(std::string const& name,
    std::string const& source, std::string const& entry,
    std::string const& target, ProgramDefines const& defines,
    unsigned int compileFlags)
{
    std::string type = target.substr(0, 3);
    if (type == "vs_" || type == "gs_" || type == "ps_" || type == "cs_")
    {
        ID3DBlob* compiledCode = CompileShader(name, source, entry, target,
            compileFlags, defines);
        if (!compiledCode)
        {
            // Errors are recorded to a logfile by CompileShader.
            return HLSLReflection();
        }

        HLSLReflection shader;
        if (!ReflectShader(name, entry, target, compiledCode, shader))
        {
            // Errors are recorded to a logfile by ReflectShader.
            shader = HLSLReflection();
        }
        DX11::SafeRelease(compiledCode);
        return shader;
    }
    else
    {
        LogError("Invalid target " + target);
    }
}

ID3DBlob* HLSLShaderFactory::CompileShader(std::string const& name,
    std::string const& source, std::string const& entry,
    std::string const& target, unsigned int compileFlags,
    ProgramDefines const& defines)
{
    ID3DInclude* include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
    ID3DBlob* compiledCode = nullptr;
    ID3DBlob* errors = nullptr;

    // Repackage the 'defines' for D3DCompile.  Prepend a definition based
    // on the engine's current matrix-vector multiplication convention.
    auto const& definitions = defines.Get();
    std::vector<D3D_SHADER_MACRO> localDefinitions(definitions.size() + 2);
    std::string umvName = "GTE_USE_MAT_VEC";
    std::string umvValue;
#if defined(GTE_USE_MAT_VEC)
    umvValue = "1";
#else
    umvValue = "0";
#endif
    localDefinitions.front().Name = umvName.c_str();
    localDefinitions.front().Definition = umvValue.c_str();
    for (size_t i = 0; i < definitions.size(); ++i)
    {
        localDefinitions[i + 1].Name = definitions[i].first.c_str();
        localDefinitions[i + 1].Definition = definitions[i].second.c_str();
    }
    localDefinitions.back().Name = nullptr;
    localDefinitions.back().Definition = nullptr;

    if (!(compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR)
    &&  !(compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR))
    {
        // The matrix storage was not explicitly set.  Use the storage
        // consistent with the CPU storage.
#if defined(GTE_USE_ROW_MAJOR)
        compileFlags |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;
#else
        compileFlags |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR;
#endif
    }

    HRESULT hr = D3DCompile(source.c_str(), source.length(), name.c_str(),
        localDefinitions.data(), include, entry.c_str(), target.c_str(),
        compileFlags, 0, &compiledCode, &errors);

    if (SUCCEEDED(hr))
    {
        if (errors)
        {
            // The message is a 'warning' from the HLSL compiler.
            char const* message =
                static_cast<char const*>(errors->GetBufferPointer());

            // Write the warning to the output window when the program is
            // executing through the Microsoft Visual Studio IDE.
            size_t const length = strlen(message);
            std::wstring output = L"";
            for (size_t i = 0; i < length; ++i)
            {
                output += static_cast<wchar_t>(message[i]);
            }
            output += L'\n';
            OutputDebugString(output.c_str());

            LogError("D3DCompile warning: " + std::string(message));
        }
    }
    else  // FAILED(hr)
    {
        if (errors)
        {
            // The message is an 'error' from the HLSL compiler.
            char const* message =
                static_cast<char const*>(errors->GetBufferPointer());
            size_t const length = strlen(message);

            // Write the error to the output window when the program is
            // executing through the Microsoft Visual Studio IDE.
            std::wstring output = L"";
            for (size_t i = 0; i < length; ++i)
            {
                output += static_cast<wchar_t>(message[i]);
            }
            output += L'\n';
            OutputDebugString(output.c_str());

            LogError("D3DCompile error: " + std::string(message));
        }
        else
        {
            // Unknown reason for error.
            LogError("D3DCompile error, hr = " + std::to_string(hr));
        }
    }

    DX11::SafeRelease(errors);
    return compiledCode;
}

bool HLSLShaderFactory::ReflectShader(std::string const& name,
    std::string const& entry, std::string const& target,
    ID3DBlob* compiledCode, HLSLReflection& shader)
{
    void const* buffer = compiledCode->GetBufferPointer();
    size_t numBytes = compiledCode->GetBufferSize();
    ID3DShaderReflection* reflector = nullptr;
    DX11Log(D3DReflect(buffer, numBytes, IID_ID3DShaderReflection, (void**)& reflector));

    bool success =
        GetDescription(reflector, shader) &&
        GetInputs(reflector, shader) &&
        GetOutputs(reflector, shader) &&
        GetCBuffers(reflector, shader) &&
        GetBoundResources(reflector, shader);

    if (success)
    {
        shader.SetName(std::string(name));
        shader.SetEntry(std::string(entry));
        shader.SetTarget(std::string(target));
        shader.SetCompiledCode(numBytes, buffer);

        if (shader.GetDescription().shaderType == D3D11_SHVER_COMPUTE_SHADER)
        {
            // The return value of GetThreadGroupSize is numX*numY*numZ, so
            // it is safe to ignore.
            UINT numX, numY, numZ;
            reflector->GetThreadGroupSize(&numX, &numY, &numZ);
            shader.SetNumThreads(numX, numY, numZ);
        }
    }

    DX11::SafeRelease(reflector);
    return success;
}

bool HLSLShaderFactory::GetDescription(ID3DShaderReflection* reflector, HLSLReflection& shader)
{
    D3D_SHADER_DESC desc;
    DX11Log(reflector->GetDesc(&desc));
    shader.SetDescription(desc);
    return true;
}

bool HLSLShaderFactory::GetInputs(ID3DShaderReflection* reflector, HLSLReflection& shader)
{
    UINT const numInputs = shader.GetDescription().numInputParameters;
    for (UINT i = 0; i < numInputs; ++i)
    {
        D3D_SIGNATURE_PARAMETER_DESC spDesc;
        DX11Log(reflector->GetInputParameterDesc(i, &spDesc));
        shader.InsertInput(HLSLParameter(spDesc));
    }
    return true;
}

bool HLSLShaderFactory::GetOutputs(ID3DShaderReflection* reflector, HLSLReflection& shader)
{
    UINT const numOutputs = shader.GetDescription().numOutputParameters;
    for (UINT i = 0; i < numOutputs; ++i)
    {
        D3D_SIGNATURE_PARAMETER_DESC spDesc;
        DX11Log(reflector->GetOutputParameterDesc(i, &spDesc));
        shader.InsertOutput(HLSLParameter(spDesc));
    }
    return true;
}

bool HLSLShaderFactory::GetCBuffers(ID3DShaderReflection* reflector, HLSLReflection& shader)
{
    UINT const numCBuffers = shader.GetDescription().numConstantBuffers;
    for (UINT i = 0; i < numCBuffers; ++i)
    {
        ID3DShaderReflectionConstantBuffer* cbuffer = reflector->GetConstantBufferByIndex(i);

        D3D_SHADER_BUFFER_DESC cbDesc;
        DX11Log(cbuffer->GetDesc(&cbDesc));

        D3D_SHADER_INPUT_BIND_DESC resDesc;
        DX11Log(reflector->GetResourceBindingDescByName(cbDesc.Name, &resDesc));

        if (cbDesc.Type == D3D_CT_CBUFFER)
        {
            std::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLConstantBuffer(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLConstantBuffer(resDesc, j, cbDesc.Size, members));
                }
            }
        }
        else if (cbDesc.Type == D3D_CT_TBUFFER)
        {
            std::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLTextureBuffer(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLTextureBuffer(resDesc, j,
                        cbDesc.Size, members));
                }
            }
        }
        else if (cbDesc.Type == D3D_CT_RESOURCE_BIND_INFO)
        {
            std::vector<HLSLBaseBuffer::Member> members;
            if (!GetVariables(cbuffer, cbDesc.Variables, members))
            {
                return false;
            }

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLResourceBindInfo(resDesc, cbDesc.Size, members));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLResourceBindInfo(resDesc, j, cbDesc.Size, members));
                }
            }
        }
        else  // cbDesc.Type == D3D_CT_INTERFACE_POINTERS
        {
            LogError("Interface pointers are not yet supported in GTEngine.");
        }
    }
    return true;
}

bool HLSLShaderFactory::GetBoundResources(ID3DShaderReflection* reflector, HLSLReflection& shader)
{
    // TODO: It appears that D3DCompile never produces a resource with a bind
    // count larger than 1.  For example, "Texture2D texture[2];" comes
    // through with names "texture[0]" and "texture[1]", each having a bind
    // count of 1 (with consecutive bind points).  So it appears that passing
    // a bind count in D3D interfaces that is larger than 1 is something a
    // programmer can do only if he manually combines the texture resources
    // into his own data structure.  If the statement about D3DCompiler is
    // true, we do not need the loops with upper bound resDesc.BindCount.

    UINT const numResources = shader.GetDescription().numBoundResources;
    for (UINT i = 0; i < numResources; ++i)
    {
        D3D_SHADER_INPUT_BIND_DESC resDesc;
        DX11Log(reflector->GetResourceBindingDesc(i, &resDesc));

        if (resDesc.Type == D3D_SIT_CBUFFER     // cbuffer
        ||  resDesc.Type == D3D_SIT_TBUFFER)    // tbuffer
        {
            // These were processed in the previous loop.
        }
        else if (
            resDesc.Type == D3D_SIT_TEXTURE        // Texture*
        ||  resDesc.Type == D3D_SIT_UAV_RWTYPED)   // RWTexture*
        {
            if (IsTextureArray(resDesc.Dimension))
            {
                if (resDesc.BindCount == 1)
                {
                    shader.Insert(HLSLTextureArray(resDesc));
                }
                else
                {
                    for (UINT j = 0; j < resDesc.BindCount; ++j)
                    {
                        shader.Insert(HLSLTextureArray(resDesc, j));
                    }
                }
            }
            else
            {
                if (resDesc.BindCount == 1)
                {
                    shader.Insert(HLSLTexture(resDesc));
                }
                else
                {
                    for (UINT j = 0; j < resDesc.BindCount; ++j)
                    {
                        shader.Insert(HLSLTexture(resDesc, j));
                    }
                }
            }
        }
        else if (resDesc.Type == D3D_SIT_SAMPLER)   // SamplerState
        {
            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLSamplerState(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLSamplerState(resDesc, j));
                }
            }
        }
        else if (
            resDesc.Type == D3D_SIT_BYTEADDRESS         // ByteAddressBuffer
        ||  resDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)  // RWByteAddressBuffer
        {
            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLByteAddressBuffer(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLByteAddressBuffer(resDesc, j));
                }
            }
        }
        else
        {
            // D3D_SIT_STRUCTURED:  StructuredBuffer
            // D3D_SIT_UAV_RWSTRUCTURED:  RWStructuredBuffer
            // D3D_SIT_UAV_APPEND_STRUCTURED:  AppendStructuredBuffer
            // D3D_SIT_UAV_CONSUME_STRUCTURED:  ConsumeStructuredBuffer
            // D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:  RWStructuredBuffer

            if (resDesc.BindCount == 1)
            {
                shader.Insert(HLSLStructuredBuffer(resDesc));
            }
            else
            {
                for (UINT j = 0; j < resDesc.BindCount; ++j)
                {
                    shader.Insert(HLSLStructuredBuffer(resDesc, j));
                }
            }
        }
    }

    return true;
}

bool HLSLShaderFactory::GetVariables(
    ID3DShaderReflectionConstantBuffer* cbuffer,
    unsigned int numVariables,
    std::vector<HLSLBaseBuffer::Member>& members)
{
    for (UINT i = 0; i < numVariables; ++i)
    {
        ID3DShaderReflectionVariable* var = cbuffer->GetVariableByIndex(i);
        ID3DShaderReflectionType* varType = var->GetType();

        D3D_SHADER_VARIABLE_DESC varDesc;
        DX11Log(var->GetDesc(&varDesc));

        D3D_SHADER_TYPE_DESC varTypeDesc;
        DX11Log(varType->GetDesc(&varTypeDesc));

        // Get the top-level information about the shader variable.
        HLSLShaderVariable svar;
        svar.SetDescription(varDesc);

        // Get the type of the variable.  If this is a struct type, the
        // call recurses to build the type tree implied by the struct.
        HLSLShaderType stype;
        stype.SetName(svar.GetName());
        stype.SetDescription(varTypeDesc);
        if (!GetTypes(varType, varTypeDesc.Members, stype))
        {
            return false;
        }

        members.push_back(std::make_pair(svar, stype));
    }
    return true;
}

bool HLSLShaderFactory::GetTypes(ID3DShaderReflectionType* rtype,
    unsigned int numMembers, HLSLShaderType& stype)
{
    for (UINT i = 0; i < numMembers; ++i)
    {
        ID3DShaderReflectionType* memType = rtype->GetMemberTypeByIndex(i);
        char const* memTypeName = rtype->GetMemberTypeName(i);
        std::string memName(memTypeName ? memTypeName : "");
        D3D_SHADER_TYPE_DESC memTypeDesc;
        DX11Log(memType->GetDesc(&memTypeDesc));

        HLSLShaderType& child = stype.GetChild(i);
        child.SetName(memName);
        child.SetDescription(memTypeDesc);
        GetTypes(memType, memTypeDesc.Members, child);
    }
    return true;
}

bool HLSLShaderFactory::IsTextureArray(D3D_SRV_DIMENSION dim)
{
    return dim == D3D_SRV_DIMENSION_TEXTURE1DARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURE2DARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURE2DMSARRAY
        || dim == D3D_SRV_DIMENSION_TEXTURECUBE
        || dim == D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
}
