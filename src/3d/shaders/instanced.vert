#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 instanceTranslation;
#ifdef USE_INSTANCE_SCALE
in vec3 instanceScale;
#endif
#ifdef USE_INSTANCE_ROTATION
in vec4 instanceRotation;
#endif

out vec3 worldPosition;
out vec3 worldNormal;

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
    // vertexPosition uses XY plane as the base plane, with Z going upwards
    // and the coordinates are local to the object

    const mat3 zUpTransform = mat3(
        // column 1
        1.0, 0.0, 0.0,
        // column 2
        0.0, 0.0, 1.0,
        // column 3
        0.0, -1.0, 0.0
    );
    // transposed inverse of z-up transform matrix
    const mat3 zUpNormalTransform = mat3(
        // column 1
        1.0, 0.0, 0.0,
        // column 2
        0.0, 0.0, 1.0,
        // column 3
        0.0, -1.0, 0.0
    );

    #ifdef USE_INSTANCE_SCALE
    vec3 thisInstanceScale = instanceScale;
    vec3 thisInstanceNormalScale = 1.0 / instanceScale;
    #else
    vec3 thisInstanceScale = symbolScale;
    vec3 thisInstanceNormalScale = 1.0 / symbolScale;
    #endif

    #ifdef USE_INSTANCE_ROTATION
    vec4 thisInstanceRotation = instanceRotation;
    #else
    vec4 thisInstanceRotation = symbolRotation;
    #endif

    // order of operations are:
    // 1. Correct for y-up to z-up
    // 2. Apply either per-instance scale or default symbol scale
    // 3. Apply either per-instance rotation or default symbol rotation
    // 4. Apply per-instance translation

    // for vertices:
    vec3 zUpPosition = zUpTransform * vertexPosition;
    vec3 scaledPosition = zUpPosition * thisInstanceScale;
    vec3 vertexPositionObject = rotateByQuat(scaledPosition, thisInstanceRotation);

    // for normals:
    vec3 zUpNormal = zUpNormalTransform * vertexNormal;
    vec3 scaledNormal = zUpNormal * thisInstanceNormalScale;
    vec3 vertexNormalObject = rotateByQuat(scaledNormal, thisInstanceRotation);

    vec3 vertexPositionChunk = vertexPositionObject + instanceTranslation;

    // Transform position and normal to world space
    worldPosition = vec3(modelMatrix * vec4(vertexPositionChunk, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormalObject);

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertexPositionChunk, 1.0);

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
