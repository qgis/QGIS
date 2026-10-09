// Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#version 330 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec4 vertexTangent;
in vec2 vertexTexCoord;

#ifdef INSTANCING
in vec3 instanceTranslation;
in vec4 instanceRotation;
in vec3 instanceScale;

vec3 rotateByQuat(vec3 v, vec4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}
#endif

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 worldTangent;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

#ifdef DATA_DEFINED_TEXTURE_TRANSFORMS
in vec4 ddTextureTransform;
#else
uniform float texCoordScale;
#ifdef TEXTURE_OFFSET
uniform vec2 texCoordOffset;
#endif
#ifdef TEXTURE_ROTATION
uniform float texCoordRotation;
#endif
#endif


#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main()
{
#ifdef INSTANCING
    vec3 pos = vertexPosition * instanceScale;
    pos = rotateByQuat(pos, instanceRotation);
    pos += instanceTranslation;

    vec3 norm = rotateByQuat(vertexNormal / instanceScale, instanceRotation);
    vec3 tang = rotateByQuat(vertexTangent.xyz * instanceScale, instanceRotation);
#else
    vec3 pos = vertexPosition;
    vec3 norm = vertexNormal;
    vec3 tang = vertexTangent.xyz;
#endif

#ifdef DATA_DEFINED_TEXTURE_TRANSFORMS
    vec2 currentTextureOffset = ddTextureTransform.xy;
    float currentTextureScale = ddTextureTransform.z;
    float currentTextureRotation = ddTextureTransform.w;
#else
    float currentTextureScale = texCoordScale;
#ifdef TEXTURE_OFFSET
    vec2 currentTextureOffset = texCoordOffset;
#else
    vec2 currentTextureOffset = vec2(0.0);
#endif
    #ifdef TEXTURE_ROTATION
    float currentTextureRotation = texCoordRotation;
    #endif
#endif

#if defined(TEXTURE_ROTATION) || defined(DATA_DEFINED_TEXTURE_TRANSFORMS)
    // handle texture rotation
    float rad = radians(currentTextureRotation);
    float c = cos(rad);
    float s = sin(rad);
    mat2 rotMat = mat2(c, s, -s, c);

    texCoord = rotMat*((vertexTexCoord- currentTextureOffset)*currentTextureScale) + currentTextureOffset;
#else
    // scale texture coordinates
    texCoord = (vertexTexCoord - currentTextureOffset) * currentTextureScale + currentTextureOffset;
#endif

    // Transform position, normal, and tangent to world space
    worldPosition = vec3(modelMatrix * vec4(pos, 1.0));
    worldNormal = normalize(modelNormalMatrix * norm);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(tang, 0.0)));
    worldTangent.w = vertexTangent.w;

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(pos, 1.0);

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
