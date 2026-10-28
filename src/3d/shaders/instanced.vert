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

// transform from mesh space (raw input vertex coordinates) to object space.
// Often this is an identity matrix, but it could be an axis flip and/or a transform
// from the gltf node of the source mesh.
uniform mat4 meshMatrix;
uniform mat3 meshNormalMatrix;

// transform from chunk space to world space (from entity's QTransform),
// applied at the very end.
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
    vec3 thisInstanceScale = instanceScale;
    #else
    vec3 thisInstanceScale = symbolScale;
    #endif

    #ifdef USE_INSTANCE_ROTATION
    vec4 thisInstanceRotation = instanceRotation;
    #else
    vec4 thisInstanceRotation = symbolRotation;
    #endif

    // order of operations are:
    // 1. mesh space to object space: apply mesh matrix
    // 2. object space to chunk space: apply per-instance scale, rotation, translation (in this order)
    // 3. chunk space to world space: apply model matrix

    // for vertices
    vec3 objectPosition = (meshMatrix * vec4(vertexPosition, 1.0)).xyz;
    vec3 chunkPosition = rotateByQuat(objectPosition * thisInstanceScale, thisInstanceRotation) + instanceTranslation;

    // for normals
    vec3 objectNormal = meshNormalMatrix * vertexNormal;
    vec3 chunkNormal = rotateByQuat(objectNormal / thisInstanceScale, thisInstanceRotation);

    // Transform position and normal to world space
    worldPosition = vec3(modelMatrix * vec4(chunkPosition, 1.0));
    worldNormal = normalize(modelNormalMatrix * chunkNormal);

#ifdef HAS_TEXTURE
    texCoord = vertexTexCoord;
#endif

#ifdef HAS_TANGENT
    vec3 chunkTangent = rotateByQuat(vertexTangent.xyz * thisInstanceScale, thisInstanceRotation);
    worldTangent.xyz = normalize(vec3(modelMatrix * vec4(chunkTangent, 0.0)));
    worldTangent.w = vertexTangent.w;
#endif

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(chunkPosition, 1.0);

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
