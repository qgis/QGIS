#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 pos;
in vec3 scale;
in vec4 rotation;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

uniform mat4 inst;  // transform of individual object instance
uniform mat4 instNormal;  // should be mat3 but Qt3D only supports mat4...
uniform vec4 symbolRotation;

uniform bool useInstanceScale;
uniform bool useInstanceRotation;

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

    vec3 instanceScaleVec;
    vec3 instanceNormalScale;
    if ( useInstanceScale )
    {
        instanceScaleVec = scale;
        instanceNormalScale = 1.0 / scale;
    }
    else
    {
        instanceScaleVec = vec3(1.0);
        instanceNormalScale = vec3(1.0);
    }

    vec4 activeSymbolRotation;
    if ( useInstanceRotation )
    {
        activeSymbolRotation = rotation;
    }
    else
    {
        activeSymbolRotation = symbolRotation;
    }

    // for vertices:
    vec3 zUpPosition = mat3(inst) * vertexPosition;
    // apply scale FIRST
    vec3 scaledPosition = zUpPosition * instanceScaleVec;
    // then rotation
    vec3 rotatedPosition = rotateByQuat(scaledPosition, activeSymbolRotation);
    // lastly, apply per instance translation
    vec3 vertexPositionObject = rotatedPosition + inst[3].xyz;

    // for normals:
    vec3 zUpNormal = mat3(instNormal) * vertexNormal;
    // apply inverse scale FIRST
    vec3 scaledNormal = zUpNormal * instanceNormalScale;
    // then rotation
    vec3 vertexNormalObject = rotateByQuat(scaledNormal, activeSymbolRotation);

    // add offset of the object relative to the chunk's origin
    vec3 vertexPositionChunk = vertexPositionObject + pos;

    // Transform position and normal to world space
    worldPosition = vec3(modelMatrix * vec4(vertexPositionChunk, 1.0));
    worldNormal = normalize(modelNormalMatrix * vertexNormalObject);

    // Calculate vertex position in clip coordinates
    gl_Position = mvp * vec4(vertexPositionChunk, 1.0);

#ifdef CLIPPING
    setClipDistance(worldPosition);
#endif
}
