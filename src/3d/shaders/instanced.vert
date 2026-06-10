#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
#ifdef HAS_TEXTURE
in vec2 vertexTexCoord;
#endif
#ifdef HAS_TANGENT
in vec4 vertexTangent;
#endif
in vec3 instanceTranslation;
#ifdef USE_INSTANCE_SCALE
in vec3 instanceScale;
#endif
#ifdef USE_INSTANCE_ROTATION
in vec4 instanceRotation;
#endif

out vec3 worldPosition;
out vec3 worldNormal;
#ifdef HAS_TEXTURE
out vec2 texCoord;
#endif
#ifdef HAS_TANGENT
out vec4 worldTangent;
#endif

uniform mat4 nodeTransform;

uniform mat3 nodeNormalTransform;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

uniform vec4 symbolRotation;
uniform vec3 symbolScale;

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

vec3 rotateByQuat(vec3 v, vec4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main()
{
    #ifdef USE_INSTANCE_SCALE
    vec3 thisScale = instanceScale;
    #else
    vec3 thisScale = symbolScale;
    #endif

    #ifdef USE_INSTANCE_ROTATION
    vec4 thisRotation = instanceRotation;
    #else
    vec4 thisRotation = symbolRotation;
    #endif

    // order of operations are:
    // 1. Apply node transform
    // 2. Apply either per-instance scale or default symbol scale
    // 3. Apply either per-instance rotation or default symbol rotation
    // 4. Apply per-instance translation

    vec3 axisPosition = (nodeTransform * vec4(vertexPosition, 1.0)).xyz;
    vec3 scaledPosition = axisPosition * thisScale;
    vec3 rotatedPosition = rotateByQuat(scaledPosition, thisRotation);
    vec3 vertexPositionChunk = rotatedPosition + instanceTranslation;

    // for normals:
    vec3 axisNormal = nodeNormalTransform * vertexNormal;
    vec3 scaledNormal = axisNormal / thisScale;
    vec3 rotatedNormal = rotateByQuat(scaledNormal, thisRotation);

    // Transform position and normal to world space
    worldPosition = vec3(modelMatrix * vec4(vertexPositionChunk, 1.0));
    worldNormal = normalize(modelNormalMatrix * rotatedNormal);

#ifdef HAS_TEXTURE
    texCoord = vertexTexCoord;
#endif

#ifdef HAS_TANGENT
    vec3 axisTangent = mat3(nodeTransform) * vertexTangent.xyz;
    vec3 scaledTangent = axisTangent * thisScale;
    vec3 rotatedTangent = rotateByQuat(scaledTangent, thisRotation);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(rotatedTangent, 0.0)));
    worldTangent.w = vertexTangent.w;
#endif

    gl_Position = mvp * vec4(vertexPositionChunk, 1.0);

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
