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

#ifdef HAS_TEXTURE
#ifdef DATA_DEFINED_TEXTURE_TRANSFORMS
in vec4 ddTextureTransform;
#elif defined(TEXTURE_ROTATION) || defined(TEXTURE_OFFSET)
uniform float texCoordScale;
#ifdef TEXTURE_OFFSET
uniform vec2 texCoordOffset;
#endif
#ifdef TEXTURE_ROTATION
uniform float texCoordRotation;
#endif
#endif
#endif

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
#ifdef DATA_DEFINED_TEXTURE_TRANSFORMS
    vec2 currentTextureOffset = ddTextureTransform.xy;
    float currentTextureScale = ddTextureTransform.z;
    float currentTextureRotation = ddTextureTransform.w;
    float rad = radians(currentTextureRotation);
    float c = cos(rad);
    float s = sin(rad);
    mat2 rotMat = mat2(c, s, -s, c);
    texCoord = rotMat * ((vertexTexCoord - currentTextureOffset) * currentTextureScale) + currentTextureOffset;
#elif defined(TEXTURE_ROTATION) || defined(TEXTURE_OFFSET)
    float currentTextureScale = texCoordScale;
#ifdef TEXTURE_OFFSET
    vec2 currentTextureOffset = texCoordOffset;
#else
    vec2 currentTextureOffset = vec2(0.0);
#endif
#ifdef TEXTURE_ROTATION
    float currentTextureRotation = texCoordRotation;
    float rad = radians(currentTextureRotation);
    float c = cos(rad);
    float s = sin(rad);
    mat2 rotMat = mat2(c, s, -s, c);
    texCoord = rotMat * ((vertexTexCoord - currentTextureOffset) * currentTextureScale) + currentTextureOffset;
#else
    texCoord = (vertexTexCoord - currentTextureOffset) * currentTextureScale + currentTextureOffset;
#endif
#else
    texCoord = vertexTexCoord;
#endif
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
