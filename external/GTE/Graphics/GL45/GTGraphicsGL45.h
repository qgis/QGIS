// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

// GL45/Engine
#include <Graphics/GL45/GL45.h>
#include <Graphics/GL45/GL45Engine.h>
#include <Graphics/GL45/GL45GraphicsObject.h>

// GL45/Engine/InputLayout
#include <Graphics/GL45/GL45InputLayout.h>
#include <Graphics/GL45/GL45InputLayoutManager.h>

// GL45/Engine/Resources
#include <Graphics/GL45/GL45Resource.h>

// GL45/Engine/Resources/Buffers
#include <Graphics/GL45/GL45AtomicCounterBuffer.h>
#include <Graphics/GL45/GL45Buffer.h>
#include <Graphics/GL45/GL45ConstantBuffer.h>
#include <Graphics/GL45/GL45IndexBuffer.h>
#include <Graphics/GL45/GL45StructuredBuffer.h>
#include <Graphics/GL45/GL45VertexBuffer.h>

// GL45/Engine/Resources/Textures
#include <Graphics/GL45/GL45DrawTarget.h>
#include <Graphics/GL45/GL45Texture.h>
#include <Graphics/GL45/GL45Texture1.h>
#include <Graphics/GL45/GL45Texture1Array.h>
#include <Graphics/GL45/GL45Texture2.h>
#include <Graphics/GL45/GL45Texture2Array.h>
#include <Graphics/GL45/GL45Texture3.h>
#include <Graphics/GL45/GL45TextureArray.h>
#include <Graphics/GL45/GL45TextureCube.h>
#include <Graphics/GL45/GL45TextureCubeArray.h>
#include <Graphics/GL45/GL45TextureDS.h>
#include <Graphics/GL45/GL45TextureRT.h>
#include <Graphics/GL45/GL45TextureSingle.h>

// GL45/Engine/State
#include <Graphics/GL45/GL45BlendState.h>
#include <Graphics/GL45/GL45DrawingState.h>
#include <Graphics/GL45/GL45DepthStencilState.h>
#include <Graphics/GL45/GL45RasterizerState.h>
#include <Graphics/GL45/GL45SamplerState.h>

// GL45/GLSL
#include <Graphics/GL45/GLSLComputeProgram.h>
#include <Graphics/GL45/GLSLProgramFactory.h>
#include <Graphics/GL45/GLSLReflection.h>
#include <Graphics/GL45/GLSLShader.h>
#include <Graphics/GL45/GLSLVisualProgram.h>

#if defined(GTE_USE_MSWINDOWS)
#include <Graphics/GL45/WGL/WGLEngine.h>
#endif

#if defined(GTE_USE_LINUX)
#include <Graphics/GL45/GLX/GLXEngine.h>
#endif
