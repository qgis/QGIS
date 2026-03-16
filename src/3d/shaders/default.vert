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
    vec3 u = q.xyz;
    float s = q.w;
    return 2.0 * dot(u, v) * u + (s*s - dot(u,u)) * v + 2.0 * s * cross(u, v);
}
#endif

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 worldTangent;
out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

uniform float texCoordScale;
#ifdef TEXTURE_ROTATION
uniform float texCoordRotation;
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

    vec3 norm = rotateByQuat(vertexNormal, instanceRotation);
    vec3 tang = rotateByQuat(vertexTangent.xyz, instanceRotation);
#else
    vec3 pos = vertexPosition;
    vec3 norm = vertexNormal;
    vec3 tang = vertexTangent.xyz;
#endif

#ifdef TEXTURE_ROTATION
    // handle texture rotation
    float rad = radians(texCoordRotation);
    float c = cos(rad);
    float s = sin(rad);
    mat2 rotMat = mat2(c, s, -s, c);

    // rotate and scale texture coordinates
    texCoord = (rotMat * vertexTexCoord) * texCoordScale;
#else
    // scale texture coordinates
    texCoord = vertexTexCoord * texCoordScale;
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
