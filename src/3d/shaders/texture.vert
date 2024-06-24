#version 150 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

out vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 mvp;

#pragma include clipplane.inc

void main()
{
    // Pass through scaled texture coordinates
    texCoord = vertexTexCoord;

    gl_Position = mvp * vec4( vertexPosition, 1.0 );

    vec3 worldPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    setClipDistance(worldPosition);
}
