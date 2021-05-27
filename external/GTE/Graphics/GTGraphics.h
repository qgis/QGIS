// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

// Base
#include <Graphics/BaseEngine.h>
#include <Graphics/GEDrawTarget.h>
#include <Graphics/GEInputLayoutManager.h>
#include <Graphics/GEObject.h>
#include <Graphics/Graphics.h>
#include <Graphics/GraphicsEngine.h>
#include <Graphics/GraphicsObject.h>

// Effects
#include <Graphics/AmbientLightEffect.h>
#include <Graphics/AreaLightEffect.h>
#include <Graphics/BumpMapEffect.h>
#include <Graphics/ConstantColorEffect.h>
#include <Graphics/CubeMapEffect.h>
#include <Graphics/DirectionalLightEffect.h>
#include <Graphics/DirectionalLightTextureEffect.h>
#include <Graphics/Font.h>
#include <Graphics/FontArialW400H12.h>
#include <Graphics/FontArialW400H14.h>
#include <Graphics/FontArialW400H16.h>
#include <Graphics/FontArialW400H18.h>
#include <Graphics/FontArialW700H12.h>
#include <Graphics/FontArialW700H14.h>
#include <Graphics/FontArialW700H16.h>
#include <Graphics/FontArialW700H18.h>
#include <Graphics/GlossMapEffect.h>
#include <Graphics/LightCameraGeometry.h>
#include <Graphics/LightEffect.h>
#include <Graphics/Lighting.h>
#include <Graphics/Material.h>
#include <Graphics/OverlayEffect.h>
#include <Graphics/PlanarReflectionEffect.h>
#include <Graphics/PointLightEffect.h>
#include <Graphics/PointLightTextureEffect.h>
#include <Graphics/ProjectedTextureEffect.h>
#include <Graphics/SphereMapEffect.h>
#include <Graphics/SpotLightEffect.h>
#include <Graphics/TextEffect.h>
#include <Graphics/Texture2Effect.h>
#include <Graphics/Texture3Effect.h>
#include <Graphics/VertexColorEffect.h>
#include <Graphics/VisualEffect.h>

// Resources
#include <Graphics/DataFormat.h>
#include <Graphics/Resource.h>

// Resources/Buffers
#include <Graphics/Buffer.h>
#include <Graphics/ConstantBuffer.h>
#include <Graphics/IndexBuffer.h>
#include <Graphics/IndexFormat.h>
#include <Graphics/IndirectArgumentsBuffer.h>
#include <Graphics/MemberLayout.h>
#include <Graphics/RawBuffer.h>
#include <Graphics/StructuredBuffer.h>
#include <Graphics/TextureBuffer.h>
#include <Graphics/TypedBuffer.h>
#include <Graphics/VertexBuffer.h>
#include <Graphics/VertexFormat.h>

// Resources/Textures
#include <Graphics/DrawTarget.h>
#include <Graphics/Texture.h>
#include <Graphics/Texture1.h>
#include <Graphics/Texture1Array.h>
#include <Graphics/Texture2.h>
#include <Graphics/Texture2Array.h>
#include <Graphics/Texture3.h>
#include <Graphics/TextureArray.h>
#include <Graphics/TextureCube.h>
#include <Graphics/TextureCubeArray.h>
#include <Graphics/TextureDS.h>
#include <Graphics/TextureRT.h>
#include <Graphics/TextureSingle.h>

// SceneGraph
#include <Graphics/MeshFactory.h>

// SceneGraph/Controllers
#include <Graphics/BlendTransformController.h>
#include <Graphics/Controller.h>
#include <Graphics/ControlledObject.h>
#include <Graphics/IKController.h>
#include <Graphics/KeyframeController.h>
#include <Graphics/MorphController.h>
#include <Graphics/ParticleController.h>
#include <Graphics/PointController.h>
#include <Graphics/SkinController.h>
#include <Graphics/TransformController.h>

// SceneGraph/Detail
#include <Graphics/BillboardNode.h>

// SceneGraph/Hierarchy
#include <Graphics/BoundingSphere.h>
#include <Graphics/Camera.h>
#include <Graphics/Light.h>
#include <Graphics/Node.h>
#include <Graphics/Particles.h>
#include <Graphics/PVWUpdater.h>
#include <Graphics/Spatial.h>
#include <Graphics/ViewVolume.h>
#include <Graphics/ViewVolumeNode.h>
#include <Graphics/Visual.h>

// SceneGraph/Picking
#include <Graphics/Picker.h>
#include <Graphics/PickRecord.h>

// SceneGraph/Sorting
#include <Graphics/BspNode.h>

// SceneGraph/Terrain
#include <Graphics/Terrain.h>

// SceneGraph/Visibility
#include <Graphics/CullingPlane.h>
#include <Graphics/Culler.h>

// Shaders
#include <Graphics/ComputeProgram.h>
#include <Graphics/ProgramDefines.h>
#include <Graphics/ProgramFactory.h>
#include <Graphics/Shader.h>
#include <Graphics/VisualProgram.h>

// State
#include <Graphics/BlendState.h>
#include <Graphics/DepthStencilState.h>
#include <Graphics/DrawingState.h>
#include <Graphics/RasterizerState.h>
#include <Graphics/SamplerState.h>
