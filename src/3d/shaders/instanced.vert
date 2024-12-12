#version 150 core

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 pos;

out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 modelMatrix;
uniform mat3 modelNormalMatrix;
uniform mat4 mvp;

uniform mat4 inst;  // transform of individual object instance
uniform mat4 instNormal;  // should be mat3 but Qt3D only supports mat4...

#ifdef CLIPPING
    #pragma include clipplane.shaderinc
#endif

void main()
{
    // vertexPosition uses XY plane as the base plane, with Z going upwards
    // and the coordinates are local to the object

    // first let's apply user defined transform for each object (translation, rotation, scaling)
    vec3 vertexPositionObject = vec3(inst * vec4(vertexPosition, 1.0));
    vec3 vertexNormalObject = mat3(instNormal) * vertexNormal;

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
