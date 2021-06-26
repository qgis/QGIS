// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/DX11/GTGraphicsDX11PCH.h>
#include <Graphics/DX11/HLSLReflection.h>
#include <algorithm>
#include <iomanip>
using namespace gte;

HLSLReflection::HLSLReflection()
    :
    mName(""),
    mEntry(""),
    mTarget(""),
    mNumXThreads(0),
    mNumYThreads(0),
    mNumZThreads(0)
{
}

bool HLSLReflection::IsValid() const
{
    return mName != "" && mEntry != "" && mTarget != "" && mCompiledCode.size() > 0;
}

void HLSLReflection::SetDescription(D3D_SHADER_DESC const& desc)
{
    mDesc.creator = std::string(desc.Creator);
    mDesc.shaderType = static_cast<D3D_SHADER_VERSION_TYPE>(D3D_SHVER_GET_TYPE(desc.Version));
    mDesc.majorVersion = D3D11_SHVER_GET_MAJOR(desc.Version);
    mDesc.minorVersion = D3D11_SHVER_GET_MINOR(desc.Version);
    mDesc.flags = desc.Flags;
    mDesc.numConstantBuffers = desc.ConstantBuffers;
    mDesc.numBoundResources = desc.BoundResources;
    mDesc.numInputParameters = desc.InputParameters;
    mDesc.numOutputParameters = desc.OutputParameters;

    mDesc.instructions.numInstructions = desc.InstructionCount;
    mDesc.instructions.numTemporaryRegisters = desc.TempRegisterCount;
    mDesc.instructions.numTemporaryArrays = desc.TempArrayCount;
    mDesc.instructions.numDefines = desc.DefCount;
    mDesc.instructions.numDeclarations = desc.DclCount;
    mDesc.instructions.numTextureNormal = desc.TextureNormalInstructions;
    mDesc.instructions.numTextureLoad = desc.TextureLoadInstructions;
    mDesc.instructions.numTextureComparison = desc.TextureCompInstructions;
    mDesc.instructions.numTextureBias = desc.TextureBiasInstructions;
    mDesc.instructions.numTextureGradient = desc.TextureGradientInstructions;
    mDesc.instructions.numFloatArithmetic = desc.FloatInstructionCount;
    mDesc.instructions.numSIntArithmetic = desc.IntInstructionCount;
    mDesc.instructions.numUIntArithmetic = desc.UintInstructionCount;
    mDesc.instructions.numStaticFlowControl = desc.StaticFlowControlCount;
    mDesc.instructions.numDynamicFlowControl = desc.DynamicFlowControlCount;
    mDesc.instructions.numMacro = desc.MacroInstructionCount;
    mDesc.instructions.numArray = desc.ArrayInstructionCount;

    mDesc.gs.numCutInstructions = desc.CutInstructionCount;
    mDesc.gs.numEmitInstructions = desc.EmitInstructionCount;
    mDesc.gs.inputPrimitive = desc.InputPrimitive;
    mDesc.gs.outputTopology = desc.GSOutputTopology;
    mDesc.gs.maxOutputVertices = desc.GSMaxOutputVertexCount;

    mDesc.ts.numPatchConstants = desc.PatchConstantParameters;
    mDesc.ts.numGSInstances = desc.cGSInstanceCount;
    mDesc.ts.numControlPoints = desc.cControlPoints;
    mDesc.ts.inputPrimitive = desc.InputPrimitive;
    mDesc.ts.outputPrimitive = desc.HSOutputPrimitive;
    mDesc.ts.partitioning = desc.HSPartitioning;
    mDesc.ts.domain = desc.TessellatorDomain;

    mDesc.cs.numBarrierInstructions = desc.cBarrierInstructions;
    mDesc.cs.numInterlockedInstructions = desc.cInterlockedInstructions;
    mDesc.cs.numTextureStoreInstructions = desc.cTextureStoreInstructions;
}

void HLSLReflection::SetCompiledCode(size_t numBytes, void const* buffer)
{
    mCompiledCode.resize(numBytes);
    std::memcpy(&mCompiledCode[0], buffer, numBytes);
}

int HLSLReflection::GetShaderTypeIndex() const
{
    switch (mTarget[0])
    {
    case 'v':
        return 0;
    case 'g':
        return 1;
    case 'p':
        return 2;
    case 'c':
        return 3;
    default:
        LogError("Invalid shader type.");
    }
}

void HLSLReflection::SetNumThreads(unsigned int numXThreads,
    unsigned int numYThreads, unsigned int numZThreads)
{
    mNumXThreads = numXThreads;
    mNumYThreads = numYThreads;
    mNumZThreads = numZThreads;
}

void HLSLReflection::Print(std::ofstream& output) const
{
    output << mName << std::endl;
    output << mEntry << std::endl;
    output << mTarget << std::endl;
    output << std::endl;

    output << "Description:" << std::endl;
    output << "creator = " << mDesc.creator << std::endl;
    int i =  std::min((int)(mDesc.shaderType), 6);
    output << "shader type = " << msShaderType[i] << std::endl;
    output << "shader version = " << mDesc.majorVersion << "." << mDesc.minorVersion << std::endl;
    output << "compile flags =" << std::endl;
    for (i = 0; i < 20; ++i)
    {
        if (mDesc.flags & (1 << i))
        {
            output << "    " << msCompileFlags[i] << std::endl;
        }
    }
    output << "constant buffers = " << mDesc.numConstantBuffers << std::endl;
    output << "bound resources = " << mDesc.numBoundResources << std::endl;
    output << "input parameters = " << mDesc.numInputParameters << std::endl;
    output << "output parameters = " << mDesc.numOutputParameters << std::endl;
    output << std::endl;

    output << "Instructions:" << std::endl;
    output << "total instructions = " << mDesc.instructions.numInstructions << std::endl;
    output << "temporary registers = " << mDesc.instructions.numTemporaryRegisters << std::endl;
    output << "temporary arrays = " << mDesc.instructions.numTemporaryArrays << std::endl;
    output << "defines = " << mDesc.instructions.numDefines << std::endl;
    output << "declarations = " << mDesc.instructions.numDeclarations << std::endl;
    output << "texture normal = " << mDesc.instructions.numTextureNormal << std::endl;
    output << "texture load = " << mDesc.instructions.numTextureLoad << std::endl;
    output << "texture comparison = " << mDesc.instructions.numTextureComparison << std::endl;
    output << "texture bias = " << mDesc.instructions.numTextureBias << std::endl;
    output << "texture gradient = " << mDesc.instructions.numTextureGradient << std::endl;
    output << "float arithmetic = " << mDesc.instructions.numFloatArithmetic << std::endl;
    output << "signed int arithmetic = " << mDesc.instructions.numSIntArithmetic << std::endl;
    output << "unsigned int arithmetic = " << mDesc.instructions.numUIntArithmetic << std::endl;
    output << "static flow control = " << mDesc.instructions.numStaticFlowControl << std::endl;
    output << "dynamic flow control = " << mDesc.instructions.numDynamicFlowControl << std::endl;
    output << "macros = " << mDesc.instructions.numMacro << std::endl;
    output << "array = " << mDesc.instructions.numArray << std::endl;
    output << std::endl;

    output << "Geometry shader parameters:" << std::endl;
    output << "cut instructions = " << mDesc.gs.numCutInstructions << std::endl;
    output << "emit instructions = " << mDesc.gs.numEmitInstructions << std::endl;
    output << "input primitive = " << msPrimitive[mDesc.gs.inputPrimitive] << std::endl;
    output << "output topology = " << msPrimitiveTopology[mDesc.gs.outputTopology] << std::endl;
    output << "max output vertices = " << mDesc.gs.maxOutputVertices << std::endl;
    output << std::endl;

    output << "Tessellation parameters:" << std::endl;
    output << "patch constants = " << mDesc.ts.numPatchConstants << std::endl;
    output << "geometry shader instances = " << mDesc.ts.numGSInstances << std::endl;
    output << "patch constants = " << mDesc.ts.numPatchConstants << std::endl;
    output << "control points = " << mDesc.ts.numControlPoints << std::endl;
    output << "input primitive = " << msPrimitive[mDesc.ts.inputPrimitive] << std::endl;
    output << "output primitive = " << msOutputPrimitive[mDesc.ts.outputPrimitive] << std::endl;
    output << "partitioning = " << msPartitioning[mDesc.ts.partitioning] << std::endl;
    output << "domain = " << msDomain[mDesc.ts.domain] << std::endl;
    output << std::endl;

    output << "Compute shader parameters:" << std::endl;
    output << "barrier instructions = " << mDesc.cs.numBarrierInstructions << std::endl;
    output << "interlocked instructions = " << mDesc.cs.numInterlockedInstructions << std::endl;
    output << "texture store instructions = " << mDesc.cs.numTextureStoreInstructions << std::endl;
    output << std::endl;

    i = 0;
    for (auto const& obj : mInputs)
    {
        output << "Input[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mOutputs)
    {
        output << "Output[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mCBuffers)
    {
        output << "ConstantBuffer[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mTBuffers)
    {
        output << "TextureBuffer[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mSBuffers)
    {
        output << "StructuredBuffer[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mRBuffers)
    {
        output << "ByteAddressBuffer[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mTextures)
    {
        output << "Texture[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mTextureArrays)
    {
        output << "TextureArray[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mSamplerStates)
    {
        output << "SamplerState[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    i = 0;
    for (auto const& obj : mRBInfos)
    {
        output << "ResourceBinding[" << i << "]:" << std::endl;
        obj.Print(output);
        output << std::endl;
        ++i;
    }

    output << "numthreads = (" << mNumXThreads << "," << mNumYThreads
        << "," << mNumZThreads << ")" << std::endl << std::endl;

    output << "compiled code = ";
    size_t size = mCompiledCode.size();
    if (size > 0)
    {
        output << std::hex << std::endl;
        size_t j = 0;
        for (auto c : mCompiledCode)
        {
            unsigned int hc = static_cast<unsigned int>(c);
            output << "0x" << std::setw(2) << std::setfill('0') << hc;
            if ((++j % 16) == 0)
            {
                if (j != size)
                {
                    output << std::endl;
                }
            }
            else
            {
                output << ' ';
            }
        }
        output << std::dec;
    }
    else
    {
        output << "none";
    }
    output << std::endl;
}


std::string const HLSLReflection::msShaderType[] =
{
    "pixel",
    "vertex",
    "geometry",
    "domain",
    "hull",
    "compute",
    "invalid"
};

std::string const HLSLReflection::msCompileFlags[] =
{
    "D3DCOMPILE_DEBUG",
    "D3DCOMPILE_SKIP_VALIDATION",
    "D3DCOMPILE_SKIP_OPTIMIZATION",
    "D3DCOMPILE_PACK_MATRIX_ROW_MAJOR",
    "D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR",
    "D3DCOMPILE_PARTIAL_PRECISION",
    "D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT",
    "D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT",
    "D3DCOMPILE_NO_PRESHADER",
    "D3DCOMPILE_AVOID_FLOW_CONTROL",
    "D3DCOMPILE_PREFER_FLOW_CONTROL",
    "D3DCOMPILE_ENABLE_STRICTNESS",
    "D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY",
    "D3DCOMPILE_IEEE_STRICTNESS",
    "D3DCOMPILE_OPTIMIZATION_LEVEL0",
    "D3DCOMPILE_OPTIMIZATION_LEVEL3",
    "D3DCOMPILE_RESERVED16",
    "D3DCOMPILE_RESERVED17",
    "D3DCOMPILE_WARNINGS_ARE_ERRORS",
    "D3DCOMPILE_RESOURCES_MAY_ALIAS"
};

std::string const HLSLReflection::msPrimitive[] =
{
    "D3D_PRIMITIVE_UNDEFINED",
    "D3D_PRIMITIVE_POINT",
    "D3D_PRIMITIVE_LINE",
    "D3D_PRIMITIVE_TRIANGLE",
    "", "",  // 4-5 unused
    "D3D_PRIMITIVE_LINE_ADJ",
    "D3D_PRIMITIVE_TRIANGLE_ADJ",
    "D3D_PRIMITIVE_1_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_2_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_3_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_4_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_5_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_6_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_7_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_8_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_9_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_10_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_11_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_12_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_13_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_14_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_15_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_16_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_17_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_18_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_19_CONTROL_POINT_PATCH",
    "", // 27 unused (appears to be a numbering error)
    "D3D_PRIMITIVE_20_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_21_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_22_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_23_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_24_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_25_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_26_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_27_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_28_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_29_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_30_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_31_CONTROL_POINT_PATCH",
    "D3D_PRIMITIVE_32_CONTROL_POINT_PATCH"
};

std::string const HLSLReflection::msPrimitiveTopology[] =
{
    "D3D_PRIMITIVE_TOPOLOGY_UNDEFINED",
    "D3D_PRIMITIVE_TOPOLOGY_POINTLIST",
    "D3D_PRIMITIVE_TOPOLOGY_LINELIST",
    "D3D_PRIMITIVE_TOPOLOGY_LINESTRIP",
    "D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST",
    "D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP",
    "", "", "", "",  // 6-9 unused
    "D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ = 10",
    "D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ = 11",
    "D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ = 12",
    "D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ = 13",
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "",  // 14-32 unused
    "D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST",
    "D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST"
};

std::string const HLSLReflection::msOutputPrimitive[] =
{
    "D3D_TESSELLATOR_OUTPUT_UNDEFINED",
    "D3D_TESSELLATOR_OUTPUT_POINT",
    "D3D_TESSELLATOR_OUTPUT_LINE",
    "D3D_TESSELLATOR_OUTPUT_TRIANGLE_CW",
    "D3D_TESSELLATOR_OUTPUT_TRIANGLE_CCW"
};

std::string const HLSLReflection::msPartitioning[] =
{
    "D3D_TESSELLATOR_PARTITIONING_UNDEFINED",
    "D3D_TESSELLATOR_PARTITIONING_INTEGER",
    "D3D_TESSELLATOR_PARTITIONING_POW2",
    "D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD",
    "D3D_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN"
};

std::string const HLSLReflection::msDomain[] =
{
    "D3D_TESSELLATOR_DOMAIN_UNDEFINED",
    "D3D_TESSELLATOR_DOMAIN_ISOLINE",
    "D3D_TESSELLATOR_DOMAIN_TRI",
    "D3D_TESSELLATOR_DOMAIN_QUAD"
};
